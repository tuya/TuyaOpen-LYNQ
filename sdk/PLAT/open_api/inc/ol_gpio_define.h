/****************************************************************************
 *
 ****************************************************************************/
#ifndef __OL_GPIO_MAP__
#define __OL_GPIO_MAP__


typedef enum
{
    mbtk_pin_not_assigned = -1,
    mbtk_pin_1=1,  mbtk_pin_2,  mbtk_pin_3,  mbtk_pin_4,  mbtk_pin_5,  mbtk_pin_6,  mbtk_pin_7,  mbtk_pin_8,
    mbtk_pin_9,  mbtk_pin_10, mbtk_pin_11, mbtk_pin_12, mbtk_pin_13, mbtk_pin_14, mbtk_pin_15, mbtk_pin_16,
    mbtk_pin_17, mbtk_pin_18, mbtk_pin_19, mbtk_pin_20, mbtk_pin_21, mbtk_pin_22, mbtk_pin_23, mbtk_pin_24,
    mbtk_pin_25, mbtk_pin_26, mbtk_pin_27, mbtk_pin_28, mbtk_pin_29, mbtk_pin_30, mbtk_pin_31, mbtk_pin_32,
    mbtk_pin_33, mbtk_pin_34, mbtk_pin_35, mbtk_pin_36, mbtk_pin_37, mbtk_pin_38, mbtk_pin_39, mbtk_pin_40,
    mbtk_pin_41, mbtk_pin_42, mbtk_pin_43, mbtk_pin_44, mbtk_pin_45, mbtk_pin_46, mbtk_pin_47, mbtk_pin_48,
    mbtk_pin_49, mbtk_pin_50, mbtk_pin_51, mbtk_pin_52, mbtk_pin_53, mbtk_pin_54, mbtk_pin_55, mbtk_pin_56,
    mbtk_pin_57, mbtk_pin_58, mbtk_pin_59, mbtk_pin_60, mbtk_pin_61, mbtk_pin_62, mbtk_pin_63, mbtk_pin_64,
    mbtk_pin_65, mbtk_pin_66, mbtk_pin_67, mbtk_pin_68, mbtk_pin_69, mbtk_pin_70, mbtk_pin_71, mbtk_pin_72,
    mbtk_pin_73, mbtk_pin_74, mbtk_pin_75, mbtk_pin_76, mbtk_pin_77, mbtk_pin_78, mbtk_pin_79, mbtk_pin_80,
    mbtk_pin_81, mbtk_pin_82, mbtk_pin_83, mbtk_pin_84, mbtk_pin_85, mbtk_pin_86, mbtk_pin_87, mbtk_pin_88,
    mbtk_pin_89, mbtk_pin_90, mbtk_pin_91, mbtk_pin_92, mbtk_pin_93, mbtk_pin_94, mbtk_pin_95, mbtk_pin_96,
    mbtk_pin_97, mbtk_pin_98, mbtk_pin_99, mbtk_pin_100, mbtk_pin_101, mbtk_pin_102, mbtk_pin_103, mbtk_pin_104,
    mbtk_pin_105, mbtk_pin_106, mbtk_pin_107, mbtk_pin_108, mbtk_pin_109, mbtk_pin_110, mbtk_pin_111, mbtk_pin_112,
    mbtk_pin_113, mbtk_pin_114, mbtk_pin_115, mbtk_pin_116, mbtk_pin_117, mbtk_pin_118, mbtk_pin_119, mbtk_pin_120,
    mbtk_pin_121, mbtk_pin_122, mbtk_pin_123, mbtk_pin_124, mbtk_pin_125, mbtk_pin_126, mbtk_pin_127,
    mbtk_pin_max_amount
}mbtk_pin_num_enum;

typedef enum
{
    mbtk_gpio_not_assigned = -1,
    mbtk_gpio_0=0,mbtk_gpio_1,  mbtk_gpio_2,  mbtk_gpio_3,  mbtk_gpio_4,  mbtk_gpio_5,  mbtk_gpio_6,  mbtk_gpio_7,
    mbtk_gpio_8,  mbtk_gpio_9,  mbtk_gpio_10, mbtk_gpio_11, mbtk_gpio_12, mbtk_gpio_13, mbtk_gpio_14, mbtk_gpio_15,
    mbtk_gpio_16, mbtk_gpio_17, mbtk_gpio_18, mbtk_gpio_19, mbtk_gpio_20, mbtk_gpio_21, mbtk_gpio_22, mbtk_gpio_23,
    mbtk_gpio_24, mbtk_gpio_25, mbtk_gpio_26, mbtk_gpio_27, mbtk_gpio_28, mbtk_gpio_29, mbtk_gpio_30, mbtk_gpio_31,
    mbtk_gpio_32, mbtk_gpio_33, mbtk_gpio_34, mbtk_gpio_35, mbtk_gpio_36, mbtk_gpio_37, mbtk_gpio_38, mbtk_gpio_39,
    mbtk_gpio_40, mbtk_gpio_41, mbtk_gpio_42, mbtk_gpio_43, mbtk_gpio_44, mbtk_gpio_45, mbtk_gpio_46, mbtk_gpio_47,
    mbtk_gpio_48, mbtk_gpio_49, mbtk_gpio_50, mbtk_gpio_51, mbtk_gpio_52, mbtk_gpio_53, mbtk_gpio_54, mbtk_gpio_55,
    mbtk_gpio_56, mbtk_gpio_57, mbtk_gpio_58, mbtk_gpio_59, mbtk_gpio_60, mbtk_gpio_61, mbtk_gpio_62, mbtk_gpio_63,
    mbtk_gpio_64, mbtk_gpio_65, mbtk_gpio_66, mbtk_gpio_67, mbtk_gpio_68, mbtk_gpio_69, mbtk_gpio_70, mbtk_gpio_71,
    mbtk_gpio_72, mbtk_gpio_73, mbtk_gpio_74, mbtk_gpio_75, mbtk_gpio_76, mbtk_gpio_77, mbtk_gpio_78, mbtk_gpio_79,
    mbtk_gpio_80, mbtk_gpio_81, mbtk_gpio_82, mbtk_gpio_83, mbtk_gpio_84, mbtk_gpio_85, mbtk_gpio_86, mbtk_gpio_87,
    mbtk_gpio_88, mbtk_gpio_89, mbtk_gpio_90, mbtk_gpio_91, mbtk_gpio_92, mbtk_gpio_93, mbtk_gpio_94, mbtk_gpio_95,
    mbtk_gpio_96, mbtk_gpio_97, mbtk_gpio_98, mbtk_gpio_99, mbtk_gpio_100, mbtk_gpio_101, mbtk_gpio_102, mbtk_gpio_103,
    mbtk_gpio_104, mbtk_gpio_105, mbtk_gpio_106, mbtk_gpio_107, mbtk_gpio_108, mbtk_gpio_109, mbtk_gpio_110, mbtk_gpio_111,
    mbtk_gpio_112, mbtk_gpio_113, mbtk_gpio_114, mbtk_gpio_115, mbtk_gpio_116, mbtk_gpio_117, mbtk_gpio_118, mbtk_gpio_119,
    mbtk_gpio_120, mbtk_gpio_121, mbtk_gpio_122, mbtk_gpio_123, mbtk_gpio_124, mbtk_gpio_125, mbtk_gpio_126, mbtk_gpio_127,
    mbtk_gpio_max_amount
}mbtk_gpio_num_enum;

typedef enum
{
    mbtk_paddr_not_assigned = -1,
    mbtk_paddr_0=0,mbtk_paddr_1,  mbtk_paddr_2,  mbtk_paddr_3,  mbtk_paddr_4,  mbtk_paddr_5,  mbtk_paddr_6,  mbtk_paddr_7,
    mbtk_paddr_8,  mbtk_paddr_9,  mbtk_paddr_10, mbtk_paddr_11, mbtk_paddr_12, mbtk_paddr_13, mbtk_paddr_14, mbtk_paddr_15,
    mbtk_paddr_16, mbtk_paddr_17, mbtk_paddr_18, mbtk_paddr_19, mbtk_paddr_20, mbtk_paddr_21, mbtk_paddr_22, mbtk_paddr_23,
    mbtk_paddr_24, mbtk_paddr_25, mbtk_paddr_26, mbtk_paddr_27, mbtk_paddr_28, mbtk_paddr_29, mbtk_paddr_30, mbtk_paddr_31,
    mbtk_paddr_32, mbtk_paddr_33, mbtk_paddr_34, mbtk_paddr_35, mbtk_paddr_36, mbtk_paddr_37, mbtk_paddr_38, mbtk_paddr_39,
    mbtk_paddr_40, mbtk_paddr_41, mbtk_paddr_42, mbtk_paddr_43, mbtk_paddr_44, mbtk_paddr_45, mbtk_paddr_46, mbtk_paddr_47,
    mbtk_paddr_48, mbtk_paddr_49, mbtk_paddr_50, mbtk_paddr_51, mbtk_paddr_52, mbtk_paddr_53, mbtk_paddr_54, mbtk_paddr_55,
    mbtk_paddr_56, mbtk_paddr_57, mbtk_paddr_58, mbtk_paddr_59, mbtk_paddr_60, mbtk_paddr_61, mbtk_paddr_62, mbtk_paddr_63,
    mbtk_paddr_64, mbtk_paddr_65, mbtk_paddr_66, mbtk_paddr_67, mbtk_paddr_68, mbtk_paddr_69, mbtk_paddr_70, mbtk_paddr_71,
    mbtk_paddr_72, mbtk_paddr_73, mbtk_paddr_74, mbtk_paddr_75, mbtk_paddr_76, mbtk_paddr_77, mbtk_paddr_78, mbtk_paddr_79,
    mbtk_paddr_80, mbtk_paddr_81, mbtk_paddr_82, mbtk_paddr_83, mbtk_paddr_84, mbtk_paddr_85, mbtk_paddr_86, mbtk_paddr_87,
    mbtk_paddr_88, mbtk_paddr_89, mbtk_paddr_90, mbtk_paddr_91, mbtk_paddr_92, mbtk_paddr_93, mbtk_paddr_94, mbtk_paddr_95,
    mbtk_paddr_96, mbtk_paddr_97, mbtk_paddr_98, mbtk_paddr_99, mbtk_paddr_100, mbtk_paddr_101, mbtk_paddr_102, mbtk_paddr_103,
    mbtk_paddr_104, mbtk_paddr_105, mbtk_paddr_106, mbtk_paddr_107, mbtk_paddr_108, mbtk_paddr_109, mbtk_paddr_110, mbtk_paddr_111,
    mbtk_paddr_112, mbtk_paddr_113, mbtk_paddr_114, mbtk_paddr_115, mbtk_paddr_116, mbtk_paddr_117, mbtk_paddr_118, mbtk_paddr_119,
    mbtk_paddr_120, mbtk_paddr_121, mbtk_paddr_122, mbtk_paddr_123, mbtk_paddr_124, mbtk_paddr_125, mbtk_paddr_126, mbtk_paddr_127,
    mbtk_paddr_max_amount
}mbtk_paddr_num_enum;

typedef struct
{
    mbtk_pin_num_enum mbtk_pin_num;
    mbtk_gpio_num_enum mbtk_gpio_num;
    mbtk_paddr_num_enum mbtk_paddr_num;
}mbtk_pin_map_struct;

#endif/*__OL_GPIO_MAP__*/
