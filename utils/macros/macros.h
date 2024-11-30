//
// Created by ionut on 29.06.2024.
//

#ifndef DSP_MACROS_H
#define DSP_MACROS_H

#define CHOOSE_MACRO(p_Macro, ...) p_Macro
#define CHOOSE_MACRO1(p_1, p_Macro, ...) CHOOSE_MACRO(p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO2(p_1, p_2, p_Macro, ...) CHOOSE_MACRO1(p_2, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO3(p_1, p_2, p_3, p_Macro, ...) CHOOSE_MACRO2(p_2, p_3, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO4(p_1, p_2, p_3, p_4, p_Macro, ...) CHOOSE_MACRO3(p_2, p_3, p_4, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO5(p_1, p_2, p_3, p_4, p_5, p_Macro, ...) CHOOSE_MACRO4(p_2, p_3, p_4, p_5, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO6(p_1, p_2, p_3, p_4, p_5, p_6, p_Macro, ...) \
        CHOOSE_MACRO5(p_2, p_3, p_4, p_5, p_6, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO7(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_Macro, ...) \
        CHOOSE_MACRO6(p_2, p_3, p_4, p_5, p_6, p_7, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO8(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_Macro, ...) \
        CHOOSE_MACRO7(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO9(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_Macro, ...) \
        CHOOSE_MACRO8(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_Macro, __VA_ARGS__)
#define CHOOSE_MACRO10(p_1, p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_10, p_Macro, ...) \
        CHOOSE_MACRO9(p_2, p_3, p_4, p_5, p_6, p_7, p_8, p_9, p_10, p_Macro, __VA_ARGS__)

#endif //DSP_MACROS_H
