/*
 * Custom_String.c
 *
 *  Created on: May 6, 2025
 *      Author: haidoan2098
 */

#include "Custom_String.h"

uint16_t getLengStr(char *str)
{
    uint16_t leng = 0;
    while (str[leng] != '\0')
    {
        leng++;
    }
    return leng;
}

//uint16_t getAtoi(char *str)
//{
//    uint16_t leng = 0;
//    uint32_t result = 0;
//
//    if (str[leng] == '\0') return 0;
//
//    while ('0' <= str[leng] && str[leng] <= '9')
//    {
//        result = result * 10 + (str[leng] - '0');
//        leng++;
//    }
//
//    return result;
//}

uint8_t strCompare(char *originalStr, char *str)
{
    if (getLengStr(originalStr) != getLengStr(str)) return 0;

    uint8_t index = 0;

    while (index != getLengStr(originalStr))
    {
        if (originalStr[index] != str[index])
        {
            return 0;
        }
        index++;
    }

    return 1;
}

