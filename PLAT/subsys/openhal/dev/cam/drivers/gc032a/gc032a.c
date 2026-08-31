#include "sctdef.h"
#include "gc032a.h"
#include "hal_cam.h"
SensorFuncObj_t gc032a_sns_func_obj;
uint32_t gc032A_2ddr_15fps_reg_list[] = {
    /*System*/
    0xffff03e8,
    0xfffe0064,
    0x00f30083,  // ff//1f//01 data output
    0x00f50008,
    0x00f70001,
    0x00f80001,  ////hai: from 0x4->0x1   pll-div----frank
    0x00f9004e,
    0x00fa0000,
    0x00fc0002,
    0x00fe0002,
    0x00810003,

    0x00fe0000,
    0x00770064,
    0x00780040,
    0x00790060,

    /*Analog&Cisctl*/
    0x00fe0000,
    0x00030001,
    0x000400cb,
    0x00050001,
    0x000600b2,
    0x00070000,
    0x00080010,

    0x000a0000,
    0x000c0000,
    0x000d0001,
    0x000e00e8,
    0x000f0002,
    0x00100088,

    0x00170054,
    0x00190008,
    0x001a000a,
    0x001f0040,
    0x00200030,
    0x002e0080,
    0x002f002b,
    0x0030001a,

    0x00fe0002,
    0x00030002,
    0x000500d7,
    0x00060060,
    0x00080080,
    0x00120089,

    /*SPI*/
    0x00fe0003,
    0x005200ba,  // hai: from 0xba -> 0x3A
    0x00530024,
    0x00540020,
    0x00550000,
    0x0059001f,  // {0x59,0x10}, 20190627 scaler output error
    0x005a0000,  // 00 //yuv hai: from 0x40 to 0x00
    0x005b0080,
    0x005c0002,
    0x005d00e0,
    0x005e0001,
    0x00510003,
    0x00640004,
    0x00fe0000,

    /*blk*/
    0x00fe0000,
    0x00180002,
    0x00fe0002,
    0x00400022,
    0x00450000,
    0x00460000,
    0x00490020,
    0x004b003c,
    0x00500020,
    0x00420010,

    /*isp*/
    0x00fe0001,
    0x000a00c5,
    0x00450000,
    0x00fe0000,
    0x004000ff,
    0x00410025,
    0x004200ef,
    0x00430010,
    0x00440083,  // hai from 0x82 to 0x83
    0x00460022,
    0x00490003,
    0x00520002,
    0x00540000,
    0x00fe0002,
    0x002200f6,

    /*Shading*/
    0x00fe0001,
    0x00c10038,
    0x00c2004c,
    0x00c30000,
    0x00c4002c,
    0x00c50024,
    0x00c60018,
    0x00c70028,
    0x00c80011,
    0x00c90015,
    0x00ca0020,
    0x00dc007a,
    0x00dd00a0,
    0x00de0080,
    0x00df0088,

    /*AWB*/ /*20170110*/
    0x00fe0001,
    0x005000c1,
    0x00560034,
    0x00580004,
    0x00650006,
    0x0066000f,
    0x00670004,
    0x00690020,
    0x006a0040,
    0x006b0081,
    0x006d0012,
    0x006e00c0,
    0x007b002a,
    0x007c000c,
    0x00fe0001,
    0x009000e3,
    0x009100c2,
    0x009200ff,
    0x009300e3,
    0x0095001c,
    0x009600ff,
    0x00970044,
    0x0098001c,
    0x009a0044,
    0x009b001c,
    0x009c0064,
    0x009d0044,
    0x009f0071,
    0x00a00064,
    0x00a10000,
    0x00a20000,
    0x00860000,
    0x00870000,
    0x00880000,
    0x00890000,
    0x00a400c2,
    0x00a5009b,
    0x00a600c8,
    0x00a70092,
    0x00a900c9,
    0x00aa0096,
    0x00ab00a9,
    0x00ac0099,
    0x00ae00ce,
    0x00af00a9,
    0x00b000cf,
    0x00b1009d,
    0x00b300cf,
    0x00b400ac,
    0x00b50000,
    0x00b60000,
    0x008b0000,
    0x008c0000,
    0x008d0000,
    0x008e0000,
    0x00940055,
    0x009900a6,
    0x009e00aa,
    0x00a3000a,
    0x008a0000,
    0x00a80055,
    0x00ad0055,
    0x00b20055,
    0x00b70005,
    0x008f0000,
    0x00b800c7,
    0x00b900a0,

    0x00fe0001,
    0x00d00040,
    0x00d10000,
    0x00d20000,
    0x00d300fa,
    0x00d4004a,
    0x00d50002,

    0x00d60044,
    0x00d700fa,
    0x00d80004,
    0x00d90008,
    0x00da005c,
    0x00db0002,
    0x00fe0000,

    /*Gamma*/
    0x00fe0000,
    0x00ba0010,
    0x00bb0012,
    0x00bc0016,
    0x00bd001b,
    0x00be0029,
    0x00bf0037,
    0x00c00046,
    0x00c10054,
    0x00c2006b,
    0x00c30079,
    0x00c40085,
    0x00c50090,
    0x00c6009a,
    0x00c700aa,
    0x00c800b8,
    0x00c900c3,
    0x00ca00cc,
    0x00cb00d3,
    0x00cc00db,
    0x00cd00e9,
    0x00ce00f6,
    0x00cf00ff,

    /*Auto Gamma*/
    0x00fe0000,
    0x005a0009,
    0x005b0014,
    0x005c0019,
    0x005d001f,
    0x005e0026,
    0x005f0032,
    0x00600045,
    0x00610053,
    0x00620069,
    0x0063007d,
    0x0064008f,
    0x0065009d,
    0x006600a9,
    0x006700bd,
    0x006800cd,
    0x006900d9,
    0x006a00e3,
    0x006b00ea,
    0x006c00ef,
    0x006d00f5,
    0x006e00f9,
    0x006f00ff,

    /*Gain*/
    0x00fe0000,
    0x00700050,

    /*AEC*/
    0x00fe0000,
    0x004f0001,
    0x00fe0001,
    0x000c0001,
    0x000d0000,  // 08 add 20170110
    0x001200a0,
    0x00130048,  // 4a   3a  hai: from 0x28 to 0x38
    0x00440004,
    0x001f0040,
    0x00200040,
    0x0023000a,
    0x0026009a,
    0x003e0020,
    0x003f002d,
    0x00400040,
    0x0041005b,
    0x00420082,
    0x004300b7,
    0x0004000a,
    0x00020079,
    0x000300c0,

    /*measure window*/
    0x00fe0001,
    0x00cc0008,
    0x00cd0008,
    0x00ce00a4,
    0x00cf00ec,

    /*DNDD*/
    0x00fe0000,
    0x008100b8,
    0x00820004,
    0x00830010,
    0x00840001,
    0x00860050,
    0x00870018,
    0x00880010,
    0x00890070,
    0x008a0020,
    0x008b0010,
    0x008c0008,
    0x008d000a,

    /*Intpee*/
    0x00fe0000,
    0x008f00aa,
    0x0090001c,
    0x00910052,
    0x00920003,
    0x00930003,
    0x00940008,
    0x009500aa,
    0x00970000,
    0x00980000,

    /*ASDE*/
    0x00fe0000,
    0x009a0030,
    0x009b0050,
    0x00a10030,
    0x00a20066,
    0x00a40028,
    0x00a50030,
    0x00aa0028,
    0x00ac0032,

    /*YCP*/
    0x00fe0000,
    0x00d1003f,
    0x00d2003f,
    0x00d30038,
    0x00d600f4,
    0x00d7001d,
    0x00dd0072,
    0x00de0084,

    0x00fe0000,
    0x00050001,
    0x000600ad,
    0x00070000,
    0x00080010,

    0x00fe0001,
    0x00250000,
    0x0026004d,

    0x00270001,
    0x002800ce,  // 16.6fps
    0x00290001,
    0x002a00ce,  // 12.5fps
    0x002b0001,
    0x002c00ce,  // 10fps
    0x002d0001,
    0x002e00ce,  // 8.33fps
    0x002f0001,
    0x003000ce,  // 5.88fps
    0x00310001,
    0x003200ce,  // 4.34fps
    0x00330001,
    0x003400ce,  // 3.99fps
    0x003c0010,  //{0x3c,0x00}
    0x00fe0000,
};

uint32_t gc032A_2ddr_30fps_reg_list[] = {
    0xffff0032,
    0xfffe0032,
    0x00f30083,
    0x00f5000c,
    0x00f70001,
    0x00f80003,
    0x00f9004e,
    0x00fa0000,
    0x00fc0002,
    0x00fe0002,
    0x00810003,
    0x00fe0000,
    0x00770064,
    0x00780040,
    0x00790060,
    /*Analog&Cisctl*/
    0x00fe0000,
    0x00030001,
    0x000400ce,
    0x00050001,
    0x000600ad,
    0x00070000,
    0x00080010,
    0x000a0000,
    0x000c0000,
    0x000d0001,
    0x000e00e8,
    0x000f0002,
    0x00100088,
    0x00170054,
    0x00190008,
    0x001a000a,
    0x001f0040,
    0x00200030,
    0x002e0080,
    0x002f002b,
    0x0030001a,
    0x00fe0002,
    0x00030002,
    0x000500d7,
    0x00060060,
    0x00080080,
    0x00120089,
    /*blk*/
    0x00fe0000,
    0x00180002,
    0x00fe0002,
    0x00400022,
    0x00450000,
    0x00460000,
    0x00490020,
    0x004b003c,
    0x00500020,
    0x00420010,
    /*isp*/
    0x00fe0001,
    0x000a00c5,
    0x00450000,
    0x00fe0000,
    0x004000ff,
    0x00410025,
    0x004200ef,
    0x00430010,
    0x00440083,
    0x00460022,
    0x00490003,
    0x00520002,
    0x00540000,
    0x00fe0002,
    0x002200f6,
    /*Shading*/
    0x00fe0001,
    0x00c10038,
    0x00c2004c,
    0x00c30000,
    0x00c4002c,
    0x00c50024,
    0x00c60018,
    0x00c70028,
    0x00c80011,
    0x00c90015,
    0x00ca0020,
    0x00dc007a,
    0x00dd00a0,
    0x00de0080,
    0x00df0088,
    /*AWB*/ /*20170110*/
    0x00fe0001,
    0x007c0009,
    0x00650006,
    0x007c0008,
    0x005600f4,
    0x0066000f,
    0x00670084,
    0x006b0080,
    0x006d0012,
    0x006e00b0,
    0x00fe0001,
    0x00900000,
    0x00910000,
    0x009200f4,
    0x009300d5,
    0x0095000f,
    0x009600f4,
    0x0097002d,
    0x0098000f,
    0x009a002d,
    0x009b000f,
    0x009c0059,
    0x009d002d,
    0x009f0067,
    0x00a00059,
    0x00a10000,
    0x00a20000,
    0x00860000,
    0x00870000,
    0x00880000,
    0x00890000,
    0x00a40000,
    0x00a50000,
    0x00a600d4,
    0x00a7009f,
    0x00a900d4,
    0x00aa009f,
    0x00ab00ac,
    0x00ac009f,
    0x00ae00d4,
    0x00af00ac,
    0x00b000d4,
    0x00b100a3,
    0x00b300d4,
    0x00b400ac,
    0x00b50000,
    0x00b60000,
    0x008b0000,
    0x008c0000,
    0x008d0000,
    0x008e0000,
    0x00940050,
    0x009900a6,
    0x009e00aa,
    0x00a3000a,
    0x008a0000,
    0x00a80050,
    0x00ad0055,
    0x00b20055,
    0x00b70005,
    0x008f0000,
    0x00b800b3,
    0x00b900b6,
    /*CC*/
    0x00fe0001,
    0x00d00040,
    0x00d100f8,
    0x00d20000,
    0x00d300fa,
    0x00d40045,
    0x00d50002,
    0x00d60030,
    0x00d700fa,
    0x00d80008,
    0x00d90008,
    0x00da0058,
    0x00db0002,
    0x00fe0000,
     /*Gamma*/
    0x00fe0000,
    0x00ba0010,
    0x00bb0012,
    0x00bc0016,
    0x00bd001b,
    0x00be0029,
    0x00bf0037,
    0x00c00046,
    0x00c10054,
    0x00c2006b,
    0x00c30079,
    0x00c40085,
    0x00c50090,
    0x00c6009a,
    0x00c700aa,
    0x00c800b8,
    0x00c900c3,
    0x00ca00cc,
    0x00cb00d3,
    0x00cc00db,
    0x00cd00e9,
    0x00ce00f6,
    0x00cf00ff,

    /*Auto Gamma*/
    0x00fe0000,
    0x005a0009,
    0x005b0014,
    0x005c0019,
    0x005d001f,
    0x005e0026,
    0x005f0032,
    0x00600045,
    0x00610053,
    0x00620069,
    0x0063007d,
    0x0064008f,
    0x0065009d,
    0x006600a9,
    0x006700bd,
    0x006800cd,
    0x006900d9,
    0x006a00e3,
    0x006b00ea,
    0x006c00ef,
    0x006d00f5,
    0x006e00f9,
    0x006f00ff,

    /*Gain*/
    0x00fe0000,
    0x00700050,
    /*AEC*/
    0x00fe0000,
    0x004f0001,
    0x00fe0001,
    0x000d0000,  // 08 add 20170110
    0x001200a0,
    0x00130048,
    0x00440004,
    0x001f0040,
    0x00200040,
    0x0026009a,
    0x003c0000,  // add
    0x003e0020,
    0x003f002d,
    0x00400040,
    0x0041005b,
    0x00420082,
    0x004300b7,
    0x0004000a,
    0x00020079,
    0x000300c0,
    /*measure window*/
    0x00fe0001,
    0x00cc0008,
    0x00cd0008,
    0x00ce00a4,
    0x00cf00ec,
    /*DNDD*/
    0x00fe0000,
    0x008100b8,
    0x00820004,
    0x00830010,
    0x00840001,
    0x00860050,
    0x00870018,
    0x00880010,
    0x00890070,
    0x008a0020,
    0x008b0010,
    0x008c0008,
    0x008d000a,
    /*Intpee*/
    0x00fe0000,
    0x008f00aa,
    0x0090001c,
    0x00910052,
    0x00920003,
    0x00930003,
    0x00940008,
    0x009500aa,
    0x00970000,
    0x00980000,
    /*ASDE*/
    0x00fe0000,
    0x009a0030,
    0x009b0050,
    0x00a10030,
    0x00a20066,
    0x00a40028,
    0x00a50030,
    0x00aa0028,
    0x00ac0032,
    /*YCP*/
    0x00fe0000,
    0x00d1003f,
    0x00d2003f,
    0x00d30038,
    0x00d600f4,
    0x00d7001d,
    0x00dd0072,
    0x00de0084,
    0x00500001,
    0x00510000,
    0x00520002,
    0x00530000,
    0x00540000,
    0x00550001,
    0x005600e0,
    0x00570002,
    0x00580080,
    /*SPI*/
    0x00fe0003,
    0x00510003,
    0x005200ba,  // 78 DDR Enable gavin 20160820
    0x00530024,  // a4//24
    0x00540020,
    0x00550000,
    0x0059001F,
    0x005a0000,
    0x005b0080,
    0x005c0002,
    0x005d00e0,
    0x005e0001,
    0x00510003,
    0x00640004,
    // 0x64,0x06 //SCK Always ON gavin 20160820
    0x00fe0000,
};

uint32_t gc032A_2sdr_reg_list[] = {
    0xffff03E8,
    0xfffe0064,
    0x00f30083,
    0x00f5000c,
    0x00f70001,  // gavin 20160820
    0x00f80001,
    0x00f9004e,
    0x00fa0010,  // gavin 20160820
    0x00fc0002,
    0x00fe0002,
    0x00810003,
    0x00fe0000,
    0x00770064,
    0x00780040,
    0x00790060,
    /*A00nag0Cictl*/
    0x00fe0000,
    0x00030001,
    0x000400c2,
    0x00050001,
    0x000600b8,
    0x00070000,
    0x00080008,
    0x000a0000,
    0x000c0000,
    0x000d0001,
    0x000e00e8,
    0x000f0002,
    0x00100088,
    0x00170054,
    0x00190008,
    0x001a000a,
    0x001f0040,
    0x00200030,
    0x002e0080,
    0x002f002b,
    0x0030001a,
    0x00fe0002,
    0x00030002,
    0x000500d7,
    0x00060060,
    0x00080080,
    0x00120089,

    /*SPI*/
    0x00fe0003,
    0x00510001,
    0x00520058,  // 0x58: ddr disable, LSB; 0xd8: ddr disable, MSB
    0x00530024,  // Disable CRC
    0x00540020,
    0x00550000,
    0x00590010,
    0x005a0000,
    0x005b0080,
    0x005c0002,
    0x005d00e0,
    0x005e0001,
    0x00640004,  // SCK Always OFF , gavin 20160820

    /*blk*/
    0x00fe0000,
    0x00180002,
    0x00fe0002,
    0x00400022,
    0x00450000,
    0x00460000,
    0x00490020,
    0x004b003c,
    0x00500020,
    0x00420010,

    /*isp*/
    0x00fe0001,
    0x000a00c5,
    0x00450000,
    0x00fe0000,
    0x004000ff,
    0x00410025,
    0x004200cf,
    0x00430010,
    0x00440083,
    0x00460022,
    0x00490003,
    0x00520002,
    0x00540000,
    0x00fe0002,
    0x002200f6,

    /*Shading*/
    0x00fe0001,
    0x00c10038,
    0x00c2004c,
    0x00c30000,
    0x00c40032,
    0x00c50024,
    0x00c60016,
    0x00c70008,
    0x00c80008,
    0x00c90000,
    0x00ca0020,
    0x00dc008a,
    0x00dd00a0,
    0x00de00a6,
    0x00df0075,

    /*AWB*/ /*20170110*/
    0x00fe0001,
    0x007c0009,
    0x00650006,
    0x007c0008,
    0x005600f4,
    0x0066000f,
    0x00670084,
    0x006b0080,
    0x006d0012,
    0x006e00b0,
    0x00fe0001,
    0x00900000,
    0x00910000,
    0x009200f4,
    0x009300d5,
    0x0095000f,
    0x009600f4,
    0x0097002d,
    0x0098000f,
    0x009a002d,
    0x009b000f,
    0x009c0059,
    0x009d002d,
    0x009f0067,
    0x00a00059,
    0x00a10000,
    0x00a20000,
    0x00860000,
    0x00870000,
    0x00880000,
    0x00890000,
    0x00a40000,
    0x00a50000,
    0x00a600d4,
    0x00a7009f,
    0x00a900d4,
    0x00aa009f,
    0x00ab00ac,
    0x00ac009f,
    0x00ae00d4,
    0x00af00ac,
    0x00b000d4,
    0x00b100a3,
    0x00b300d4,
    0x00b400ac,
    0x00b50000,
    0x00b60000,
    0x008b0000,
    0x008c0000,
    0x008d0000,
    0x008e0000,
    0x00940050,
    0x009900a6,
    0x009e00aa,
    0x00a3000a,
    0x008a0000,
    0x00a80050,
    0x00ad0055,
    0x00b20055,
    0x00b70005,
    0x008f0000,
    0x00b800b3,
    0x00b900b6,

    /*CC*/
    0x00fe0001,
    0x00d00040,
    0x00d100f8,
    0x00d20000,
    0x00d300fa,
    0x00d40045,
    0x00d50002,
    0x00d60030,
    0x00d700fa,
    0x00d80008,
    0x00d90008,
    0x00da0058,
    0x00db0002,
    0x00fe0000,

    /*Gamma*/
    0x00fe0000,
    0x00ba0000,
    0x00bb0004,
    0x00bc000a,
    0x00bd000e,
    0x00be0022,
    0x00bf0030,
    0x00c0003d,
    0x00c1004a,
    0x00c2005d,
    0x00c3006b,
    0x00c4007a,
    0x00c50085,
    0x00c60090,
    0x00c700a5,
    0x00c800b5,
    0x00c900c2,
    0x00ca00cc,
    0x00cb00d5,
    0x00cc00de,
    0x00cd00ea,
    0x00ce00f5,
    0x00cf00ff,

    /*A00utG0mm*/
    0x00fe0000,
    0x005a0008,
    0x005b000f,
    0x005c0015,
    0x005d001c,
    0x005e0028,
    0x005f0036,
    0x00600045,
    0x00610051,
    0x0062006a,
    0x0063007d,
    0x0064008d,
    0x00650098,
    0x006600a2,
    0x006700b5,
    0x006800c3,
    0x006900cd,
    0x006a00d4,
    0x006b00dc,
    0x006c00e3,
    0x006d00f0,
    0x006e00f9,
    0x006f00ff,

    /*Gain*/
    0x00fe0000,
    0x00700050,

    /*AEC*/
    0x00fe0000,
    0x004f0001,
    0x00fe0001,
    0x000d0000,  // 08 add 20170110
    0x001200a0,
    0x0013003a,
    0x00440004,
    0x001f0030,
    0x00200040,
    0x0026009a,
    0x003e0020,
    0x003f002d,
    0x00400040,
    0x0041005b,
    0x00420082,
    0x004300b7,
    0x0004000a,
    0x00020079,
    0x000300c0,

    /*measure window*/
    0x00fe0001,
    0x00cc0008,
    0x00cd0008,
    0x00ce00a4,
    0x00cf00ec,

    /*DNDD*/
    0x00fe0000,
    0x008100b8,
    0x00820012,
    0x0083000a,
    0x00840001,
    0x00860050,
    0x00870018,
    0x00880010,
    0x00890070,
    0x008a0020,
    0x008b0010,
    0x008c0008,
    0x008d000a,

    /*Intpee*/
    0x00fe0000,
    0x008f00aa,
    0x0090009c,
    0x00910052,
    0x00920003,
    0x00930003,
    0x00940008,
    0x00950044,
    0x00970000,
    0x00980000,

    /*ASDE*/
    0x00fe00000,
    0x00a100030,
    0x00a200041,
    0x00a400030,
    0x00a500020,
    0x00aa00030,
    0x00ac00032,

    /*YCP*/
    0x00fe0000,
    0x00d1003c,
    0x00d2003c,
    0x00d30038,
    0x00d600f4,
    0x00d7001d,
    0x00dd0073,
    0x00de0084,
    0x00fe0000,
    0x00050001,
    0x000600ad,
    0x00070000,
    0x00080010,

    0x00fe0001,
    0x00250000,
    0x0026004d,
    0x00270001,
    0x002800ce,
    0x00290003,
    0x002a0002,
    0x002b0003,
    0x002c009c,
    0x002d0004,
    0x002e0036,
    0x002f0004,
    0x003000d0,
    0x00310005,
    0x0032006a,
    0x00330006,
    0x00340004,
    0x00fe0000,
};

uint32_t gc032A_1sdr_reg_list[] = {
    0xffff03E8, 0xfffe0064,
    0x00f30083,  // sync_output_en data_output_en
    0x00f5000c, 0x00f70001, 0x00f80001, 0x00f9004e, 0x00fa0030, 0x00fc0002,
    0x00fe0002,
    0x00810003,  // doesn't show 0x81 addr
    /*Analog*/
    0x00fe0000,
    0x00030001,  // exposure high
    0x000400c2,  // exposure low
    0x00050001, 0x00060091, 0x00070000, 0x00080010, 0x000a0004, 0x000c0004,
    0x000d0001,
    0x000e00e8,  // height: 488
    0x000f0002,
    0x00100088,  // width: 652    VGA: 640*480
    0x00170054, 0x00190004, 0x001a000a, 0x001f0040, 0x00200030, 0x002e0080,
    0x002f002b, 0x0030001a, 0x00fe0002, 0x00030002, 0x000500d7, 0x00060060,
    0x00080080, 0x00120089,
    /*blk*/
    0x00fe0000, 0x00180002, 0x00fe0002, 0x00400022, 0x00450000, 0x00460000,
    0x00490020, 0x004b003c, 0x00500020, 0x00420010,
    /*isp*/
    0x00fe0001, 0x000a00c5, 0x00450000, 0x00fe0000, 0x004000ff, 0x00410025,
    0x00420083, 0x00430010, 0x00440083, 0x00460026, 0x00490003, 0x004f0001,
    0x00de0084, 0x00fe0002, 0x002200f6,
    /*Shading*/
    0x00fe0000, 0x00770065, 0x00780040, 0x00790052, 0x00fe0001, 0x00c1003c,
    0x00c20050, 0x00c30000, 0x00c40032, 0x00c50024, 0x00c60016, 0x00c70008,
    0x00c80008, 0x00c90000, 0x00ca0020, 0x00dc007a, 0x00dd007a, 0x00de00a6,
    0x00df0060,
    /*AWB*/
    0x00fe0001, 0x007c0009, 0x00650006, 0x007c0008, 0x005600f4, 0x0066000f,
    0x00670084, 0x006b0080, 0x006d0012, 0x006e00b0, 0x00860000, 0x00870000,
    0x00880000, 0x00890000, 0x008a0000, 0x008b0000, 0x008c0000, 0x008d0000,
    0x008e0000, 0x008f0000, 0x009000ef, 0x009100e1, 0x0092000c, 0x009300ef,
    0x00940065, 0x0095001f, 0x0096000c, 0x0097002d, 0x00980020, 0x009900aa,
    0x009a003f, 0x009b002c, 0x009c005f, 0x009d003e, 0x009e00aa, 0x009f0067,
    0x00a00060, 0x00a10000, 0x00a20000, 0x00a3000a, 0x00a400b6, 0x00a500ac,
    0x00a600c1, 0x00a700ac, 0x00a80055, 0x00a900c3, 0x00aa00a4, 0x00ab00ba,
    0x00ac00a8, 0x00ad0055, 0x00ae00c8, 0x00af00b9, 0x00b000d4, 0x00b100c3,
    0x00b20055, 0x00b300d8, 0x00b400ce, 0x00b50000, 0x00b60000, 0x00b70005,
    0x00b800d6, 0x00b9008c,
    /*CC*/
    0x00fe0001, 0x00d00040, 0x00d100f8, 0x00d20000, 0x00d300fa, 0x00d40045,
    0x00d50002, 0x00d60030, 0x00d700fa, 0x00d80008, 0x00d90008, 0x00da0058,
    0x00db0002, 0x00fe0000,
    /*Gamma*/
    0x00fe0000, 0x00ba0000, 0x00bb0004, 0x00bc000a, 0x00bd000e, 0x00be0022,
    0x00bf0030, 0x00c0003d, 0x00c1004a, 0x00c2005d, 0x00c3006b, 0x00c4007a,
    0x00c50085, 0x00c60090, 0x00c700a5, 0x00c800b5, 0x00c900c2, 0x00ca00cc,
    0x00cb00d5, 0x00cc00de, 0x00cd00ea, 0x00ce00f5, 0x00cf00ff,
    /*Auto Ga*/
    0x00fe0000, 0x005a0008, 0x005b000f, 0x005c0015, 0x005d001c, 0x005e0028,
    0x005f0036, 0x00600045, 0x00610051, 0x0062006a, 0x0063007d, 0x0064008d,
    0x00650098, 0x006600a2, 0x006700b5, 0x006800c3, 0x006900cd, 0x006a00d4,
    0x006b00dc, 0x006c00e3, 0x006d00f0, 0x006e00f9, 0x006f00ff,
    /*Gain*/
    0x00fe0000, 0x00700050,
    /*AEC*/
    0x00fe0000, 0x004f0001, 0x00fe0001, 0x001200a0, 0x0013003a, 0x00440004,
    0x001f0030, 0x00200040, 0x00260014, 0x00270001, 0x002800f4, 0x00290002,
    0x002a003a, 0x002b0002, 0x002c00ac, 0x002d0002, 0x002e00f8, 0x002f000b,
    0x0030006e, 0x0031000e, 0x00320070, 0x00330012, 0x0034000c, 0x003c0000,
    0x003e0020, 0x003f002d, 0x00400040, 0x0041005b, 0x00420082, 0x004300b7,
    0x0004000a, 0x00020079, 0x000300c0,
    /*measure*/
    0x00cc0008, 0x00cd0008, 0x00ce00a4, 0x00cf00ec,
    /*DNDD*/
    0x00fe0000, 0x008100b8, 0x00820012, 0x0083000a, 0x00840001, 0x00860050,
    0x00870018, 0x00880010, 0x00890070, 0x008a0020, 0x008b0010, 0x008c0008,
    0x008d000a,
    /*Intpee*/
    0x00fe0000, 0x008f00aa, 0x0090009c, 0x00910052, 0x00920003, 0x00930003,
    0x00940008, 0x00950044, 0x00970000, 0x00980000,
    /*ASDE*/
    0x00fe0000, 0x00a10030, 0x00a20041, 0x00a40030, 0x00a50020, 0x00aa0030,
    0x00ac0032,
    /*YCP*/
    0x00fe0000, 0x00d1003c, 0x00d2003c, 0x00d30038, 0x00d600f4, 0x00d7001d,
    0x00dd0073, 0x00de0084,
    /*SPI*/
    0x00fe0003,  // registers in REGF3
    0x00510001,
    0x0052000a,  // Y: 266
    0x00530020,
    0x00540020,  // X: 0x220 = 544
    0x00550000, 0x00590010,
    0x005a0000,  // sync format
    0x005b0080,
    0x005c0002,  // image width:640
    0x005d00e0,
    0x005e0001,  // image height:480
};

int gc032a_get_reg_list(uint8_t mode, uint32_t **reg_list, uint32_t *count)
{
    switch(mode)
    {
        case GC032A_IMG_MODE_SDR1BIT_640X480_15FPS: {
            *reg_list = gc032A_1sdr_reg_list;
            *count = (sizeof(gc032A_1sdr_reg_list) /
                      sizeof(gc032A_1sdr_reg_list[0]));
            gc032a_sns_func_obj.max_fps = 1500;
            break;
        }
        case GC032A_IMG_MODE_SDR2BIT_640X480_15FPS: {
            *reg_list = gc032A_2sdr_reg_list;
            *count = (sizeof(gc032A_2sdr_reg_list) /
                      sizeof(gc032A_2sdr_reg_list[0]));
            gc032a_sns_func_obj.max_fps = 1500;
            break;
        }
        case GC032A_IMG_MODE_DDR2BIT_640X480_15FPS: {
            *reg_list = gc032A_2ddr_15fps_reg_list;
            *count = (sizeof(gc032A_2ddr_15fps_reg_list) /
                      sizeof(gc032A_2ddr_15fps_reg_list[0]));
            gc032a_sns_func_obj.max_fps = 1500;
            break;
        }
        case GC032A_IMG_MODE_DDR2BIT_640X480_30FPS: {
            *reg_list = gc032A_2ddr_30fps_reg_list;
            *count = (sizeof(gc032A_2ddr_30fps_reg_list) /
                      sizeof(gc032A_2ddr_30fps_reg_list[0]));
            gc032a_sns_func_obj.max_fps = 3000;
            break;
        }
        default: {
            return -1;
        }
    }
    return 0;
}

int gc032a_set_mirror_flip(uint8_t direct)
{
    uint8_t reg_value = 0;
    if(direct == 3)
    {
        reg_value = 0x57;
    }
    else if(direct == 1)
    {
        reg_value = 0x55;
    }
    else if(direct == 2)
    {
        reg_value = 0x56;
    }
    else
    {
        reg_value = 0x54;
    }
    return hal_cam_write_reg(GC032A_I2C_ADDR, 0x17, reg_value);
}

int gc032a_set_fps(uint32_t fps)
{
    uint32_t vts = GC032A_DEFAULT_VTS;
    uint32_t dummy = 0;
    int ret = 0;
    if(fps > gc032a_sns_func_obj.max_fps)
    {
        fps = gc032a_sns_func_obj.max_fps;
    }
    if(fps < GC032A_DEFAULT_MINUM_FPS)
    {
        fps = GC032A_DEFAULT_MINUM_FPS;
    }
    vts = GC032A_DEFAULT_VTS * gc032a_sns_func_obj.max_fps / fps;
    dummy = vts - GC032A_DEFAULT_START_OFFSET_LINE_NUMS -
            GC032A_DEFAULT_ALL_LINE_NUMS;
    ret = hal_cam_write_reg(GC032A_I2C_ADDR, 0x07,
                            (uint8_t)((dummy & 0xFF00) >> 8));
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x08, (uint8_t)(dummy & 0x00FF));
    return ret;
}

int gc032a_get_sensor_id(uint32_t *id)
{
    uint8_t idH = 0;
    uint8_t idL = 0;
    hal_cam_read_reg(GC032A_I2C_ADDR, 0xf0, &idH);
    hal_cam_read_reg(GC032A_I2C_ADDR, 0xf1, &idL);
    *id = (((uint32_t)idH) << 8) + idL;
    return 0;
}

int gc032a_set_ev(uint8_t ev)
{
    uint8_t evValue = GC032A_DEFAULT_EV_VALUE;
    int ret = 0;
    ret = hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x01);
    if(ev >= 50)
    {
        evValue =
            (GC032A_MAX_EV_VALUE - GC032A_DEFAULT_EV_VALUE) * (ev - 50) / 50 +
            GC032A_DEFAULT_EV_VALUE;
    }
    else
    {
        evValue = GC032A_MIN_EV_VALUE +
                  ((GC032A_DEFAULT_EV_VALUE - GC032A_MIN_EV_VALUE) * ev / 50);
    }
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x13, evValue);
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x00);
    return ret;
}

int gc032a_set_contrast(uint8_t contrast)
{
    uint8_t conValue = GC032A_DEFAULT_CONTRAST_VALUE;

    if(contrast >= 50)
    {
        conValue = (GC032A_MAX_CONTRAST_VALUE - GC032A_DEFAULT_CONTRAST_VALUE) *
                       (contrast - 50) / 50 +
                   GC032A_DEFAULT_CONTRAST_VALUE;
    }
    else
    {
        conValue =
            GC032A_MIN_CONTRAST_VALUE +
            ((GC032A_DEFAULT_CONTRAST_VALUE - GC032A_MIN_CONTRAST_VALUE) *
             contrast / 50);
    }
    return hal_cam_write_reg(GC032A_I2C_ADDR, 0xd3, conValue);
}

int gc032a_set_saturation(uint8_t sat)
{
    int ret = 0;
    uint8_t satValue = GC032A_DEFAULT_SATURATION_VALUE;

    if(sat >= 50)
    {
        satValue =
            (GC032A_MAX_SATURATION_VALUE - GC032A_DEFAULT_SATURATION_VALUE) *
                (sat - 50) / 50 +
            GC032A_DEFAULT_SATURATION_VALUE;
    }
    else
    {
        satValue =
            GC032A_MIN_SATURATION_VALUE +
            ((GC032A_DEFAULT_SATURATION_VALUE - GC032A_MIN_SATURATION_VALUE) *
             sat / 50);
    }
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xd1, satValue);
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xd2, satValue);
    return ret;
}

int gc032a_set_sharpness(uint8_t sharp)
{
    uint8_t sharpValue = GC032A_DEFAULT_SHARPEN_VALUE;
    uint8_t sharpEdge1 = (sharpValue & 0xF0) >> 4;
    uint8_t sharpEdge2 = sharpValue & 0x0F;
    if(sharp >= 50)
    {
        sharpEdge1 =
            (sharp - 50) *
                (((GC032A_MAX_SHARPEN_VALUE & 0xF0) >> 4) - sharpEdge1) / 50 +
            ((GC032A_DEFAULT_SHARPEN_VALUE & 0xF0) >> 4);
        sharpEdge2 = (sharp - 50) *
                         ((GC032A_MAX_SHARPEN_VALUE & 0x0F) - sharpEdge2) / 50 +
                     (GC032A_DEFAULT_SHARPEN_VALUE & 0x0F);
    }
    else
    {
        sharpEdge1 =
            ((GC032A_MIN_SHARPEN_VALUE & 0xF0) >> 4) +
            sharp * (sharpEdge1 - ((GC032A_MIN_SHARPEN_VALUE & 0xF0) >> 4)) /
                50;
        sharpEdge2 =
            (GC032A_MIN_SHARPEN_VALUE & 0x0F) +
            sharp * (sharpEdge2 - (GC032A_MIN_SHARPEN_VALUE & 0x0F)) / 50;
    }
    return hal_cam_write_reg(GC032A_I2C_ADDR, 0x95,
                             (sharpEdge1 << 4) + sharpEdge2);
}

int gc032a_set_awb(bool awbEnable, uint8_t scene)
{
    int ret = 0;
    uint8_t reg_value = 0;
    ret = hal_cam_read_reg(GC032A_I2C_ADDR, 0x42, &reg_value);
    if(ret != 0)
    {
        return ret;
    }
    uint8_t rgain = 0;
    uint8_t ggain = 0;
    uint8_t bgain = 0;
    switch(scene)
    {
        case CAM_WB_AUTO:
            reg_value |= 0x02;
            break;
        case CAM_WB_CLOUD:
            reg_value &= ~0x02;
            rgain = 0x8c;
            ggain = 0x50;
            bgain = 0x40;
            break;
        case CAM_WB_DAYLIGHT:
            reg_value &= ~0x02;
            rgain = 0x74;
            ggain = 0x52;
            bgain = 0x40;
            break;
        case CAM_WB_INCANDESCENCE:
            reg_value &= ~0x02;
            rgain = 0x48;
            ggain = 0x40;
            bgain = 0x5c;
            break;
        case CAM_WB_FLUORESCENT:
            reg_value &= ~0x02;
            rgain = 0x40;
            ggain = 0x42;
            bgain = 0x50;
            break;
        case CAM_WB_TUNGSTEN:
            reg_value &= ~0x02;
            rgain = 0x40;
            ggain = 0x54;
            bgain = 0x70;
            break;
        default:
            return -1;
    }
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x42, reg_value);
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x77, rgain);
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x78, ggain);
    ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x79, bgain);
    return ret;
}

int gc032a_set_gamma(uint8_t *table, uint32_t size)
{
    int ret = 0;
    if(size < GC032A_GAMMA_TABLE_SIZE)
    {
        return -1;
    }
    for(int i = 0; i < GC032A_GAMMA_TABLE_SIZE; i++)
    {
        ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x5a + i, *(table + i));
    }
    return ret;
}

int gc032a_set_ae(uint8_t aeMode)
{
    return hal_cam_write_reg(GC032A_I2C_ADDR, 0x4f, aeMode == 0 ? 0x01 : 0x00);
}

int gc032a_set_scene(uint8_t scene)
{
    int ret = 0;
    switch(scene)
    {
        case CAM_SCENE_DAY: {
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x27, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x28, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x29, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2a, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2b, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2c, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2d, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2e, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2f, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x30, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x31, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x32, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x33, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x34, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x3c, 0x10);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x00);
            break;
        }
        case CAM_SCENE_NIGHT: {
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x27, 0x01);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x28, 0xce);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x29, 0x02);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2a, 0x12);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2b, 0x03);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2c, 0x50);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2d, 0x05);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2e, 0xcc);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x2f, 0x04);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x30, 0x74);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x31, 0x09);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x32, 0x1c);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x33, 0x0c);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x34, 0x20);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0x3c, 0x30);
            ret |= hal_cam_write_reg(GC032A_I2C_ADDR, 0xfe, 0x00);
            break;
        }
        default:
            return -1;
    }
    return ret;
}

int gc032a_power_down(void)
{
    hal_cam_write_reg(GC032A_I2C_ADDR, 0xf3, 0x00);
    return 0;
}

int gc032a_power_up(void)
{
    hal_cam_write_reg(GC032A_I2C_ADDR, 0xf3, 0x83);
    return 0;
}

CamCfg_t gc032a_default_cfg = {
    .drv_id = CSPI_1,
    .int_mode = GC032A_IMG_MODE_DDR2BIT_640X480_15FPS,
    .mclk_freq = CAM_24_M,
    .reso = CAM_8W_COLOR,
    .img_out_wnd = false,
    .wnd_cfg = {.start_x = 0, .start_y = 0, .width = 0, .height = 0},
    .seq_cfg.clock_delay = 2,
    .spi_cfg =
        {
            .endianMode = CAM_MSB_MODE,
            .wireNum = WIRE_2,
            .rxSeq = SEQ_1,
            .cpol = 0,
            .cpha = 1,
            .ddrMode = 1,
            .wordIdSeq = 1,
            .yOnly = 0,
            .rowScaleRatio = 1,
            .colScaleRatio = 1,
            .scaleBytes = 3,
            .dummyAllowed = 1,
        },
    .ext_pwr_cfg =
        {
            .enable = 1,
            .pad_num = 11,
            .mux = PAD_MUX_ALT4,
            .io_num = 16,
        },
    .rst_pin_cfg =
        {
            .pad_num = 20,
            .mux = PAD_MUX_ALT0,
            .io_num = 5,
            .reset_level = 1,
        },
    .i2c_cfg =
        {
            .i2c_port = 0,
            .speed = OPEN_I2C_SPEED_100KHZ,
        },
    .qbuf_cfg = {
        .item_count = 3,
        .item_size = 320 * 240 * 2,
        .pool_addr = NULL,
    }
};

SensorFuncObj_t gc032a_sns_func_obj = {
    .type = CAM_TYPE_GC032A,
    .sensor_id = 0x232a,
    .img_width = 640,
    .img_height = 480,
    .max_fps = 1500,  // actural fps * 100
    .dev_addr = GC032A_I2C_ADDR,
    .reg_addr_size = 1,
    .reg_data_size = 1,
    .default_cfg = &gc032a_default_cfg,
    .pfn_set_mirror_flip = gc032a_set_mirror_flip,
    .pfn_set_fps = gc032a_set_fps,
    .pfn_get_sensor_id = gc032a_get_sensor_id,
    .pfn_set_ev = gc032a_set_ev,
    .pfn_set_contrast = gc032a_set_contrast,
    .pfn_set_saturation = gc032a_set_saturation,
    .pfn_set_sharp = gc032a_set_sharpness,
    .pfn_set_awb = gc032a_set_awb,
    .pfn_set_gamma = gc032a_set_gamma,
    .pfn_set_ae = gc032a_set_ae,
    .pfn_set_scene = gc032a_set_scene,
    .pfn_get_init_reg_list = gc032a_get_reg_list,
    .pfn_power_down = gc032a_power_down,
    .pfn_power_up = gc032a_power_up,
};
