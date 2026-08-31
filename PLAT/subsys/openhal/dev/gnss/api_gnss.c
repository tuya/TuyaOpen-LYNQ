/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_gnss.c
 * Description:  ec7xx openHAL Gnss source file
 * History:      Rev1.0   2024-11-12
 *
 ****************************************************************************/
#include <stdlib.h>
#include "api_gnss.h"
#include <stdarg.h>
#include <string.h>

#define boolstr(s) ((s) ? "true" : "false")

int hex2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

bool minmea_check(const char *sentence, bool strict)
{
    uint8_t checksum = 0x00;

    // A valid sentence starts with "$".
    if (*sentence == '$')
        sentence++;

    // The optional checksum is an XOR of all bytes between "$" and "*".
    while (*sentence && *sentence != '*' && isprint((unsigned char)*sentence))
        checksum ^= *sentence++;

    // If checksum is present...
    if (*sentence == '*')
    {
        // Extract checksum.
        sentence++;
        int upper = hex2int(*sentence++);
        if (upper == -1)
            return false;
        int lower = hex2int(*sentence++);
        if (lower == -1)
            return false;
        int expected = upper << 4 | lower;

        // Check for checksum mismatch.
        if (checksum != expected)
            return false;
    }
    else if (strict)
    {
        // Discard non-checksummed frames in strict mode.
        return false;
    }

    // The only stuff allowed at this point is a newline.
    while (*sentence == '\r' || *sentence == '\n')
    {
        sentence++;
    }

    if (*sentence)
    {
        return false;
    }

    return true;
}

bool minmea_scan(const char *sentence, const char *format, ...)
{
    bool result = false;
    bool optional = false;

    if (sentence == NULL)
        return false;

    va_list ap;
    va_start(ap, format);

    const char *field = sentence;
#define next_field()                            \
    do                                          \
    {                                           \
        /* Progress to the next field. */       \
        while (minmea_isfield(*sentence))       \
            sentence++;                         \
        /* Make sure there is a field there. */ \
        if (*sentence == ',')                   \
        {                                       \
            sentence++;                         \
            field = sentence;                   \
        }                                       \
        else                                    \
        {                                       \
            field = NULL;                       \
        }                                       \
    } while (0)

    while (*format)
    {
        char type = *format++;

        if (type == ';')
        {
            // All further fields are optional.
            optional = true;
            continue;
        }

        if (!field && !optional)
        {
            // Field requested but we ran out if input. Bail out.
            goto parse_error;
        }

        switch (type)
        {
        case 'c':
        { // Single character field (char).
            char value = '\0';

            if (field && minmea_isfield(*field))
                value = *field;

            *va_arg(ap, char *) = value;
        }
        break;

        case 'd':
        { // Single character direction field (int).
            int value = 0;

            if (field && minmea_isfield(*field))
            {
                switch (*field)
                {
                case 'N':
                case 'E':
                    value = 1;
                    break;
                case 'S':
                case 'W':
                    value = -1;
                    break;
                default:
                    goto parse_error;
                }
            }

            *va_arg(ap, int *) = value;
        }
        break;

        case 'f':
        { // Fractional value with scale (struct minmea_float).
            int sign = 0;
            int_least32_t value = -1;
            int_least32_t scale = 0;

            if (field)
            {
                while (minmea_isfield(*field))
                {
                    if (*field == '+' && !sign && value == -1)
                    {
                        sign = 1;
                    }
                    else if (*field == '-' && !sign && value == -1)
                    {
                        sign = -1;
                    }
                    else if (isdigit((unsigned char)*field))
                    {
                        int digit = *field - '0';
                        if (value == -1)
                            value = 0;
                        if (value > (INT_LEAST32_MAX - digit) / 10)
                        {
                            /* we ran out of bits, what do we do? */
                            if (scale)
                            {
                                /* truncate extra precision */
                                break;
                            }
                            else
                            {
                                /* integer overflow. bail out. */
                                goto parse_error;
                            }
                        }
                        value = (10 * value) + digit;
                        if (scale)
                            scale *= 10;
                    }
                    else if (*field == '.' && scale == 0)
                    {
                        scale = 1;
                    }
                    else if (*field == ' ')
                    {
                        /* Allow spaces at the start of the field. Not NMEA
                         * conformant, but some modules do this. */
                        if (sign != 0 || value != -1 || scale != 0)
                            goto parse_error;
                    }
                    else
                    {
                        goto parse_error;
                    }
                    field++;
                }
            }

            if ((sign || scale) && value == -1)
                goto parse_error;

            if (value == -1)
            {
                /* No digits were scanned. */
                value = 0;
                scale = 0;
            }
            else if (scale == 0)
            {
                /* No decimal point. */
                scale = 1;
            }
            if (sign)
                value *= sign;

            *va_arg(ap, struct minmea_float *) = (struct minmea_float){value, scale};
        }
        break;

        case 'i':
        { // Integer value, default 0 (int).
            int value = 0;

            if (field)
            {
                char *endptr;
                value = strtol(field, &endptr, 10);
                if (minmea_isfield(*endptr))
                    goto parse_error;
            }

            *va_arg(ap, int *) = value;
        }
        break;

        case 's':
        { // String value (char *).
            char *buf = va_arg(ap, char *);

            if (field)
            {
                while (minmea_isfield(*field))
                    *buf++ = *field++;
            }

            *buf = '\0';
        }
        break;

        case 't':
        { // NMEA talker+sentence identifier (char *).
            // This field is always mandatory.
            if (!field)
                goto parse_error;

            if (field[0] != '$')
                goto parse_error;
            for (int f = 0; f < 5; f++)
                if (!minmea_isfield(field[1 + f]))
                    goto parse_error;

            char *buf = va_arg(ap, char *);
            memcpy(buf, field + 1, 5);
            buf[5] = '\0';
        }
        break;

        case 'D':
        { // Date (int, int, int), -1 if empty.
            struct minmea_date *date = va_arg(ap, struct minmea_date *);

            int d = -1, m = -1, y = -1;

            if (field && minmea_isfield(*field))
            {
                // Always six digits.
                for (int f = 0; f < 6; f++)
                    if (!isdigit((unsigned char)field[f]))
                        goto parse_error;

                char dArr[] = {field[0], field[1], '\0'};
                char mArr[] = {field[2], field[3], '\0'};
                char yArr[] = {field[4], field[5], '\0'};
                d = strtol(dArr, NULL, 10);
                m = strtol(mArr, NULL, 10);
                y = strtol(yArr, NULL, 10);
            }

            date->day = d;
            date->month = m;
            date->year = y;
        }
        break;

        case 'T':
        { // Time (int, int, int, int), -1 if empty.
            struct minmea_time *time_ = va_arg(ap, struct minmea_time *);

            int h = -1, i = -1, s = -1, u = -1;

            if (field && minmea_isfield(*field))
            {
                // Minimum required: integer time.
                for (int f = 0; f < 6; f++)
                    if (!isdigit((unsigned char)field[f]))
                        goto parse_error;

                char hArr[] = {field[0], field[1], '\0'};
                char iArr[] = {field[2], field[3], '\0'};
                char sArr[] = {field[4], field[5], '\0'};
                h = strtol(hArr, NULL, 10);
                i = strtol(iArr, NULL, 10);
                s = strtol(sArr, NULL, 10);
                field += 6;

                // Extra: fractional time. Saved as microseconds.
                if (*field++ == '.')
                {
                    uint32_t value = 0;
                    uint32_t scale = 1000000LU;
                    while (isdigit((unsigned char)*field) && scale > 1)
                    {
                        value = (value * 10) + (*field++ - '0');
                        scale /= 10;
                    }
                    u = value * scale;
                }
                else
                {
                    u = 0;
                }
            }

            time_->hours = h;
            time_->minutes = i;
            time_->seconds = s;
            time_->microseconds = u;
        }
        break;

        case '_':
        { // Ignore the field.
        }
        break;

        default:
        { // Unknown.
            goto parse_error;
        }
        }

        next_field();
    }

    result = true;

parse_error:
    va_end(ap);
    return result;
}

enum minmea_sentence_id minmea_sentence_id(const char *sentence, bool strict)
{
    if (!minmea_check(sentence, strict))
        return MINMEA_INVALID;

    char type[6];
    if (!minmea_scan(sentence, "t", type))
        return MINMEA_INVALID;

    if (!strcmp(type + 2, "GBS"))
        return MINMEA_SENTENCE_GBS;
    if (!strcmp(type + 2, "GGA"))
        return MINMEA_SENTENCE_GGA;
    if (!strcmp(type + 2, "GLL"))
        return MINMEA_SENTENCE_GLL;
    if (!strcmp(type + 2, "GSA"))
        return MINMEA_SENTENCE_GSA;
    if (!strcmp(type + 2, "GST"))
        return MINMEA_SENTENCE_GST;
    if (!strcmp(type + 2, "GSV"))
        return MINMEA_SENTENCE_GSV;
    if (!strcmp(type + 2, "RMC"))
        return MINMEA_SENTENCE_RMC;
    if (!strcmp(type + 2, "VTG"))
        return MINMEA_SENTENCE_VTG;
    if (!strcmp(type + 2, "ZDA"))
        return MINMEA_SENTENCE_ZDA;
    if (!strcmp(type + 2, "TXT"))
        return MINMEA_SENTENCE_TXT;
    return MINMEA_UNKNOWN;
}

bool minmea_parse_gbs(struct minmea_sentence_gbs *frame, const char *sentence)
{
    // $GNGBS,170556.00,3.0,2.9,8.3,,,,*5C
    char type[6];
    if (!minmea_scan(sentence, "tTfffifff",
                     type,
                     &frame->time,
                     &frame->err_latitude,
                     &frame->err_longitude,
                     &frame->err_altitude,
                     &frame->svid,
                     &frame->prob,
                     &frame->bias,
                     &frame->stddev))
        return false;
    if (strcmp(type + 2, "GBS"))
        return false;

    return true;
}

// $GNRMC,071209.00,A,3110.94352,N,12136.37477,E,0.193,,070723,,,A,V*1E
// 0[0,69]$GNRMC,063116.00,A,3110.96251,N,12136.35722,E,0.069,,100723,,,A,V*13
bool minmea_parse_rmc(struct minmea_sentence_rmc *frame, const char *sentence)
{
    // $GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62
    char validity;
    int latitude_direction;
    int longitude_direction;
    int variation_direction;
    if (!minmea_scan(sentence, "tTcfdfdffDfd",
                     frame->type,
                     &frame->time,
                     &validity,
                     &frame->latitude, &latitude_direction,
                     &frame->longitude, &longitude_direction,
                     &frame->speed,
                     &frame->course,
                     &frame->date,
                     &frame->variation, &variation_direction))
        return false;
    if (strcmp(frame->type + 2, "RMC"))
        return false;

    frame->valid = (validity == 'A');
    frame->latitude.value *= latitude_direction;
    frame->longitude.value *= longitude_direction;
    frame->variation.value *= variation_direction;

    return true;
}

bool minmea_parse_gga(struct minmea_sentence_gga *frame, const char *sentence)
{
    // $GNGGA,084852.000,2236.9453,N,11408.4790,E,1,05,3.1,89.7,M,0.0,M,,*48
    // $GPGGA,123519    ,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    int latitude_direction;
    int longitude_direction;
    if (!minmea_scan(sentence, "tTfdfdiiffcfcf_",
                     frame->type,
                     &frame->time,
                     &frame->latitude, &latitude_direction,
                     &frame->longitude, &longitude_direction,
                     &frame->fix_quality,
                     &frame->satellites_tracked,
                     &frame->hdop,
                     &frame->altitude, &frame->altitude_units,
                     &frame->height, &frame->height_units, // 天线大地高
                     &frame->dgps_age))
        return false;
    if (strcmp(frame->type + 2, "GGA"))
        return false;

    frame->latitude.value *= latitude_direction;
    frame->longitude.value *= longitude_direction;

    return true;
}
// $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
// 2[138,69]:$GNGSA,A,3,06,13,15,195,05,29,30,199,07,11,20,18,1.54,0.87,1.27,1*07
// 3[207,47]:$GNGSA,A,3,05,11,,,,,,,,,,,1.54,0.87,1.27,3*0D
// 4[254,55]:$GNGSA,A,3,67,82,88,66,81,68,,,,,,,1.54,0.87,1.27,2*05
bool minmea_parse_gsa(struct minmea_sentence_gsa *frame, const char *sentence)
{
    if (!minmea_scan(sentence, "tciiiiiiiiiiiiifff",
                     frame->type,
                     &frame->mode,
                     &frame->fix_type,
                     &frame->sats[0],
                     &frame->sats[1],
                     &frame->sats[2],
                     &frame->sats[3],
                     &frame->sats[4],
                     &frame->sats[5],
                     &frame->sats[6],
                     &frame->sats[7],
                     &frame->sats[8],
                     &frame->sats[9],
                     &frame->sats[10],
                     &frame->sats[11],
                     &frame->pdop,
                     &frame->hdop,
                     &frame->vdop))
        return false;
    if (strcmp(frame->type + 2, "GSA"))
        return false;

    return true;
}

bool minmea_parse_gll(struct minmea_sentence_gll *frame, const char *sentence)
{
    // $GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41$;

    int latitude_direction;
    int longitude_direction;

    if (!minmea_scan(sentence, "tfdfdTc;c",
                     frame->type,
                     &frame->latitude, &latitude_direction,
                     &frame->longitude, &longitude_direction,
                     &frame->time,
                     &frame->status,
                     &frame->mode))
        return false;
    if (strcmp(frame->type + 2, "GLL"))
        return false;

    frame->latitude.value *= latitude_direction;
    frame->longitude.value *= longitude_direction;

    return true;
}

bool minmea_parse_gst(struct minmea_sentence_gst *frame, const char *sentence)
{
    // $GPGST,024603.00,3.2,6.6,4.7,47.3,5.8,5.6,22.0*58
    char type[6];

    if (!minmea_scan(sentence, "tTfffffff",
                     type,
                     &frame->time,
                     &frame->rms_deviation,
                     &frame->semi_major_deviation,
                     &frame->semi_minor_deviation,
                     &frame->semi_major_orientation,
                     &frame->latitude_error_deviation,
                     &frame->longitude_error_deviation,
                     &frame->altitude_error_deviation))
        return false;
    if (strcmp(type + 2, "GST"))
        return false;

    return true;
}

bool minmea_parse_gsv(struct minmea_sentence_gsv *frame, const char *sentence)
{
    if (!minmea_scan(sentence, "tiii;iiiiiiiiiiiiiiii",
                     frame->type,
                     &frame->total_msgs,
                     &frame->msg_nr,
                     &frame->total_sats,
                     &frame->sats[0].nr,
                     &frame->sats[0].elevation,
                     &frame->sats[0].azimuth,
                     &frame->sats[0].snr,
                     &frame->sats[1].nr,
                     &frame->sats[1].elevation,
                     &frame->sats[1].azimuth,
                     &frame->sats[1].snr,
                     &frame->sats[2].nr,
                     &frame->sats[2].elevation,
                     &frame->sats[2].azimuth,
                     &frame->sats[2].snr,
                     &frame->sats[3].nr,
                     &frame->sats[3].elevation,
                     &frame->sats[3].azimuth,
                     &frame->sats[3].snr))
    {
        return false;
    }
    if (strcmp(frame->type + 2, "GSV"))
        return false;

    return true;
}

bool minmea_parse_vtg(struct minmea_sentence_vtg *frame, const char *sentence)
{
    // $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
    // $GPVTG,156.1,T,140.9,M,0.0,N,0.0,K*41
    // $GPVTG,096.5,T,083.5,M,0.0,N,0.0,K,D*22
    // $GPVTG,188.36,T,,M,0.820,N,1.519,K,A*3F

    char c_true, c_magnetic, c_knots, c_kph, c_faa_mode;

    if (!minmea_scan(sentence, "t;fcfcfcfcc",
                     frame->type,
                     &frame->true_track_degrees,
                     &c_true,
                     &frame->magnetic_track_degrees,
                     &c_magnetic,
                     &frame->speed_knots,
                     &c_knots,
                     &frame->speed_kph,
                     &c_kph,
                     &c_faa_mode))
        return false;
    if (strcmp(frame->type + 2, "VTG"))
        return false;
    // values are only valid with the accompanying characters
    if (c_true != 'T')
        frame->true_track_degrees.scale = 0;
    if (c_magnetic != 'M')
        frame->magnetic_track_degrees.scale = 0;
    if (c_knots != 'N')
        frame->speed_knots.scale = 0;
    if (c_kph != 'K')
        frame->speed_kph.scale = 0;
    frame->faa_mode = (enum minmea_faa_mode)c_faa_mode;

    return true;
}

bool minmea_parse_zda(struct minmea_sentence_zda *frame, const char *sentence)
{
    // $GPZDA,201530.00,04,07,2002,00,00*60
    if (!minmea_scan(sentence, "tTiiiii",
                     frame->type,
                     &frame->time,
                     &frame->date.day,
                     &frame->date.month,
                     &frame->date.year,
                     &frame->hour_offset,
                     &frame->minute_offset))
        return false;
    if (strcmp(frame->type + 2, "ZDA"))
        return false;

    // check offsets
    if (abs(frame->hour_offset) > 13 ||
        frame->minute_offset > 59 ||
        frame->minute_offset < 0)
        return false;

    return true;
}

bool minmea_parse_txt(struct minmea_sentence_txt *frame, const char *sentence)
{
    // $GPTXT,01,01,01,ANTENNA OK*35
    if (!minmea_scan(sentence, "tiiis",
                     frame->type,
                     &frame->data1,
                     &frame->data2,
                     &frame->data3,
                     NULL))
        return false;
    if (strcmp(frame->type + 2, "TXT"))
        return false;
    if (strstr(sentence, "ANTENNA OK"))
        frame->flag = 1;
    return true;
}

uint16_t nmea_line_parse(char *line, gnss_data_t *gnss)
{
    // char *head = NULL;
    // char *token = NULL;
    // uint8_t ret = 0;
    // uint16_t num = 0;
    enum minmea_sentence_id type_id = minmea_sentence_id(line, false);
    switch (type_id)
    {
    case MINMEA_SENTENCE_GLL:
    {
        struct minmea_sentence_gll frame;
        if (minmea_parse_gll(&frame, line))
        {
#if GNSS_DEBUG_PRINTF
            printf("%s  %d:%d:%d:%d\r\n",
                   frame.type,
                   frame.time.hours,
                   frame.time.minutes,
                   frame.time.seconds,
                   frame.time.microseconds);
            printf("\t latitude:%d/%d,longitude %d/%d\r\n",
                   frame.latitude.value, frame.latitude.scale,
                   frame.longitude.value, frame.longitude.scale);
            printf("\t status %c, mode %c\r\n", frame.status, frame.mode);
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxGLL sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_GSA: // 当前卫星信息
    {
        struct minmea_sentence_gsa frame;
        if (minmea_parse_gsa(&frame, line))
        {
#if GNSS_DEBUG_PRINTF
            printf("%s\t[P%d,H%d,V%d]:", frame.type, frame.pdop, frame.hdop, frame.vdop);
            for (int i = 0; i < 12; i++)
            {
                if (frame.sats[i])
                {
                    printf("%d ", frame.sats[i]);
                }
            }
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxGSA sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_GGA: // 全球定位信息
    {
        struct minmea_sentence_gga frame;
        if (minmea_parse_gga(&frame, line))
        {
            gnss->altitude = frame.altitude.value;
            gnss->tracked = frame.satellites_tracked;
            gnss->quality = frame.fix_quality;
            gnss->hdop = frame.hdop.value;
            gnss->time.hours = frame.time.hours;
            gnss->time.minutes = frame.time.minutes;
            gnss->time.seconds = frame.time.seconds;
            gnss->latitude = frame.latitude.value;
            gnss->longitude = frame.longitude.value;
            gnss->latitude_scale = frame.latitude.scale;
            gnss->longitude_scale = frame.longitude.scale;
            gnss->altitude_scale = frame.altitude.scale;

#if GNSS_DEBUG_PRINTF
            printf("%s\t%d:%d:%d:%d",
                   frame.type,
                   frame.time.hours,
                   frame.time.minutes,
                   frame.time.seconds,
                   frame.time.microseconds);
            printf("\tlatitude:%d.%d,longitude:%d.%d,altitude:%d.%d\r\n",
                   (frame.latitude.value / frame.latitude.scale), (frame.latitude.value % frame.latitude.scale),
                   (frame.longitude.value / frame.longitude.scale), (frame.longitude.value % frame.longitude.scale),
                   (frame.altitude.value / frame.altitude.scale), (frame.altitude.value % frame.altitude.scale));
            printf("\thdop:%d,tracked:%d,quality:%d\r\n", frame.hdop.value, frame.satellites_tracked, frame.fix_quality);
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxGGA sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_GSV: // 可见卫星信息
    {
        struct minmea_sentence_gsv frame;
        if (minmea_parse_gsv(&frame, line))
        {
        }
    }
    break;
    case MINMEA_SENTENCE_RMC:
    {
        struct minmea_sentence_rmc frame;
        if (minmea_parse_rmc(&frame, line)) // 推荐定位信息
        {
            if (!frame.valid)
            {
                gnss->valid = 0;
                // gIsFix = false;
                return -1;
            }
            // gIsFix = true;
            gnss->valid = 1;

            // gnss->date = frame.date;
            gnss->time.year = frame.date.year;
            gnss->time.month = frame.date.month;
            gnss->time.day = frame.date.day;
            gnss->time.hours = frame.time.hours;
            gnss->time.minutes = frame.time.minutes;
            gnss->time.seconds = frame.time.seconds;
            gnss->speed = frame.speed.value;
// 在此处通知到任务
#if GNSS_DEBUG_PRINTF
            printf("%s\t%02d.%02d.%02d %d:%d:%d ",
                   frame.type,
                   frame.date.year,
                   frame.date.month,
                   frame.date.day,
                   frame.time.hours,
                   frame.time.minutes,
                   frame.time.seconds);
            // 311096596/100000,1213635808/100000,51/1000
            printf("\tlatitude:%d.%d,longitude:%d.%d,speed:%d.%d\r\n",
                   (frame.latitude.value / frame.latitude.scale), (frame.latitude.value % frame.latitude.scale),
                   (frame.longitude.value / frame.longitude.scale), (frame.longitude.value % frame.longitude.scale),
                   (frame.speed.value / frame.speed.scale), (frame.speed.value % frame.speed.scale));
            printf("scale = %d\r\n", frame.latitude.scale);

#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxRMC sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_VTG:
    {
        struct minmea_sentence_vtg frame;
        if (minmea_parse_vtg(&frame, line))
        {
#if GNSS_DEBUG_PRINTF
            printf("%s: true track degrees = %d\r\n",
                   frame.type,
                   frame.true_track_degrees);
            printf("\t magnetic track degrees = %d\r\n",
                   frame.magnetic_track_degrees);
            printf("\t speed knots = %d\r\n",
                   frame.speed_knots);
            printf("\t speed kph = %d\r\n",
                   frame.speed_kph);
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxVTG sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_ZDA:
    {
        struct minmea_sentence_zda frame;
        if (minmea_parse_zda(&frame, line))
        {
#if GNSS_DEBUG_PRINTF
            printf("%s: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d",
                   frame.type,
                   frame.time.hours,
                   frame.time.minutes,
                   frame.time.seconds,
                   frame.date.day,
                   frame.date.month,
                   frame.date.year,
                   frame.hour_offset,
                   frame.minute_offset);
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxZDA sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_SENTENCE_TXT:
    {
        struct minmea_sentence_txt frame;
        if (minmea_parse_txt(&frame, line))
        {
#if GNSS_DEBUG_PRINTF
            printf("%s: %d:%d:%d %d",
                   frame.type,
                   frame.data1,
                   frame.data2,
                   frame.data3,
                   frame.flag);
#endif
        }
        else
        {
#if GNSS_DEBUG_PRINTF
            printf("$xxTXT sentence is not parsed\r\n");
#endif
        }
    }
    break;
    case MINMEA_INVALID:
    {
#if GNSS_DEBUG_PRINTF
        printf("$xxxxx sentence is invalid\r\n");
#endif
    }
    break;
    default:
    {
#if GNSS_DEBUG_PRINTF
        printf("$xxxxx sentence is not parsed\r\n");
#endif
    }
    break;
    }
    return 0;
}

int gnss_parse(char *str, gnss_data_t *data)
{
    if (str == NULL || strlen(str) < 6)
        return 0;
    char *head = NULL;
    char *nmea_line[NMEA_ITEMS_MAX];
    int num = 0, line = 0;
    const char s[2] = "\n";
    head = strtok(str, s);
    while (head != NULL)
    {
        nmea_line[line] = head;
        num += strlen(nmea_line[line]);
        head = strtok(NULL, s);
        line++;
    }
    num += line;
    data->items = line;
    data->size = num;
    // struct minmea_sentence_gsa frame;
    for (int i = 0; i < line; i++)
    {
        nmea_line_parse(nmea_line[i], data);
    }
    return data->valid;
}
