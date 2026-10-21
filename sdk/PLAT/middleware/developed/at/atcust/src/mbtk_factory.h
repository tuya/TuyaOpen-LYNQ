#ifndef __MBTK_FACTORY_H__
#define __MBTK_FACTORY_H__


typedef enum
{
    PIN_1 = 1,
    PIN_2 = 2,
    PIN_3 = 3,
    PIN_4 = 4,
    PIN_5 = 5,
    PIN_6 = 6,
    PIN_7 = 7,
    PIN_8 = 8,
    PIN_9 = 9,
    PIN_10 = 10,
    PIN_11 = 11,
    PIN_13 = 13,
    PIN_14 = 14,
    PIN_15 = 15,
    PIN_16 = 16,
    PIN_17 = 17,
    PIN_18 = 18,
    PIN_19 = 19,
    PIN_20 = 20,
    PIN_21 = 21,
    PIN_22 = 22,
    PIN_23 = 23,
    PIN_24 = 24,
    PIN_25 = 25,
    PIN_26 = 26,
    PIN_28 = 28,
    PIN_29 = 29,
    PIN_30 = 30,
    PIN_31 = 31,
    PIN_32 = 32,
    PIN_33 = 33,
    PIN_34 = 34,
    PIN_35 = 35,
    PIN_36 = 36,
    PIN_37 = 37,
    PIN_38 = 38,
    PIN_39 = 39,
    PIN_40 = 40,
    PIN_48 = 48,
    PIN_49 = 49,
    PIN_50 = 50,
    PIN_51 = 51,
    PIN_52 = 52,
    PIN_53 = 53,
    PIN_54 = 54,
    PIN_55 = 55,
    PIN_56 = 56,
    PIN_57 = 57,
    PIN_58 = 58,
    PIN_59 = 59,
    PIN_60 = 60,
    PIN_61 = 61,
    PIN_62 = 62,
    PIN_63 = 63,
    PIN_64 = 64,
    PIN_65 = 65,
    PIN_66 = 66,
    PIN_67 = 67,
    PIN_69 = 69,
    PIN_70 = 70,
    PIN_71 = 71,
    PIN_72 = 72,
    PIN_78 = 78,
    PIN_79 = 79,
    PIN_80 = 80,
    PIN_81 = 81,
    PIN_100 = 100,
    PIN_101 = 101,
    PIN_103 = 103,
    PIN_MAX,
} mftest_pin_number;

typedef struct
{
    unsigned char com_pin;
    unsigned char com_gpio;
    unsigned char com_pad_addr;
    unsigned char com_mux;
    unsigned char adc_index;
}mbtk_gpio_adc_tab_t;

typedef struct
{
    unsigned char com_pin;
    unsigned char com_gpio;
    unsigned char com_pad_addr;
    unsigned char com_mux;
    unsigned char nc_pin;
    unsigned char nc_gpio;
    unsigned char nc_pad_addr;
    unsigned char nc_mux;
}mbtk_gpio_paired_tab_t;

typedef struct
{
    unsigned char low_fail_count;
    unsigned char low_fail_pin[PIN_MAX];
    unsigned char high_fail_count;
    unsigned char high_fail_pin[PIN_MAX];
} mbtk_gpio_test_result_t;


extern mbtk_gpio_paired_tab_t* mbtk_get_paired_gpio(int index);
extern mbtk_gpio_adc_tab_t* mbtk_get_adc_gpio(int index);
extern void mbtk_test_gpio_set_dcd(void);
extern void mbtk_test_gpio_set_wakeup(void);
extern int mbtk_check_gpio_is_wakeup(int input_pin);
extern int mbtk_check_gpio_input_wakeup(int input_pin, int check_high);
extern int mbtk_test_adc_vol(void);
extern int mbtk_check_adc_level(int adc_index, int check_high);

#endif /*__MBTK_FACTORY_H__*/