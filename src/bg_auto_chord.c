#include "bg_auto_chord.h"
#include "bg_chord_config.h"
#include <stdio.h>

const uint8_t base_tone[6] = {40, 45, 50, 55, 59, 64};

BG_ERR BG_chord_enable(uint8_t enable);
BG_ERR BG_chord_setmode(uint8_t mode);
BG_ERR BG_chord_setkey(int8_t key);
BG_ERR BG_chord_Auto_Chord(uint8_t *chord, uint8_t tone, uint8_t property);
uint8_t BG_chord_status(void);
uint8_t BG_chord_getmode(void);
uint8_t BG_chord_getkey(void);

BG_Chord BG_chord = {

    .Enable = BG_chord_enable,
    .SetMode = BG_chord_setmode,
    .SetKey = BG_chord_setkey,
    .Auto_Chord = BG_chord_Auto_Chord,
    .Status = BG_chord_status,
    .GetMode = BG_chord_getkey,

};

typedef struct _Chord_run_data
{
    uint8_t mode;
    uint8_t enable;
    uint8_t span;
    uint8_t key;
    uint8_t result_count;
    uint8_t rule_count;
    int8_t chord_rule[6];
    uint8_t chord_result[6][7];
    uint8_t last_chord;

} Chord_run_data;

Chord_run_data chord_run_data = {

    .mode = 0,
    .enable = 0,
    .span = 3,
    .last_chord = 0,
    .key = 0,
    .result_count = 0,
    .rule_count = 0,
    .chord_rule = {0},
    .chord_result = {0}

};

BG_ERR BG_chord_enable(uint8_t enable)
{

    if (enable > 1)
        return INVALID_INPUT;

    chord_run_data.enable = enable;

    return SUCCESS;
}

BG_ERR BG_chord_setkey(int8_t key)
{

    if (key < -12 || key > 12)
        return KEY_ERR;

    chord_run_data.key = key;

    return SUCCESS;
}

uint8_t BG_chord_getkey(void)
{

    return chord_run_data.key;
}

BG_ERR BG_chord_setmode(uint8_t mode)
{

    if (mode > 1)
        return INVALID_INPUT;

    chord_run_data.mode = mode;

    return SUCCESS;
}

void Get_MinMax_fret(uint8_t *chord, uint8_t *minv, uint8_t *maxv)
{
    uint8_t last_min;
    last_min = chord[chord[6]];
    *minv = chord[chord[6]];
    *maxv = chord[chord[6]];

    for (uint8_t string = chord[6]; string < 6; string++)
    {

        *minv = chord[string];
        if (*maxv < *minv)
        {

            *minv = *maxv;
            *maxv = chord[string];
            if (last_min < *minv)
                *minv = last_min;
            last_min = *minv;
#ifdef HIGH_DEBUG_LEVEL
            printf("string is %d max is %d min is  %d last_min is %d\n", string, *maxv, *minv, last_min);
#endif
        }
        else
        {
            if (last_min < *minv)
                *minv = last_min;
            last_min = *minv;
#ifdef HIGH_DEBUG_LEVEL
            printf("max is %d min is  %d last_min is %d\n", *maxv, *minv, last_min);
#endif
        }

        if (*minv > chord[chord[6]])
            *minv = chord[chord[6]];
    }
#ifdef CHORD_DEBUG
    printf("Get_MinMax_fret: max is %d min is%d\n", *maxv, *minv);
#endif
}

void Graph_chord(uint8_t *chord)
{
    uint8_t max, min;
    uint8_t fret_reflect[4] = {1, 4, 7, 10};
    Get_MinMax_fret(chord, &min, &max);
    for (uint8_t string = 6; string > 0; string--)
    {
        for (uint8_t print_count = 0; print_count < 12; print_count++)
            if (chord[string - 1] == 0xff && print_count == 1)
            {

                printf("-X-");
            }
            else if (fret_reflect[chord[string - 1] - min] == print_count && chord[string - 1] != 0xff)
            {
                printf("-%d-", chord[string - 1]);
            }
            else if (print_count == 11)
            {
                printf("---\n");
            }
            else
            {

                printf("---");
            }
        if (string > 1)
            printf("|        |        |        |        |\n");
    }
}

void OutPutChord(uint8_t *chord)
{
    for (uint8_t i = 0; i < 6; i++)
        chord[i] += base_tone[i] + chord[i];
}

void chord_generator(uint8_t *chord, uint8_t tone, uint8_t property)
{
    uint8_t low_string = 0;
    uint8_t low_string_val[3];
    uint8_t i;
    uint8_t string;
    uint8_t chord_count;
    uint8_t count;

    uint8_t loop_count;
    uint8_t check_over;
    chord_run_data.rule_count = 0;
    for (i = 0; i < 6; i++)
        if (chord_rule[property].intervallic[i] != 0)
            chord_run_data.rule_count++;
    chord_run_data.rule_count += 1;
    for (string = 0; string < 3; string++)
    {
        for (i = 0; i < 12; i++)
        {
            if (tone == (base_tone[low_string] + i) % 12)
            {

#ifdef CHORD_DEBUG
                printf("*********************GET_ROOT*******************\n");
#endif

                count = 0;
                while (count < low_string)
                {

                    chord_run_data.chord_result[chord_run_data.result_count][count] = 0xff;
                    chord_run_data.chord_result[chord_run_data.result_count + 3][count] = 0xff;
                    count++;

#ifdef CHORD_DEBUG
                    printf("The value is %d\n", chord_run_data.chord_result[chord_run_data.result_count][count]);
#endif
                }
                chord_run_data.chord_result[chord_run_data.result_count][6] = low_string;
                chord_run_data.chord_result[chord_run_data.result_count][low_string] = i;
                chord_run_data.chord_result[chord_run_data.result_count + 3][6] = low_string;
                chord_run_data.chord_result[chord_run_data.result_count + 3][low_string] = i;
#ifdef HIGH_DEBUG_LEVEL
                printf("The value2 is %d\n", chord_run_data.chord_result[chord_run_data.result_count][count]);
#endif

                // Update low string's value;
                low_string_val[count] = chord_run_data.chord_result[chord_run_data.result_count][count];
                chord_run_data.result_count++;

#ifdef CHORD_DEBUG
                printf("The Chord count is %d ,string is %d, val is %d \n", chord_run_data.result_count, low_string, i);
#endif
            }
        }

        low_string++;
    }

    count = 0;
    chord_run_data.chord_rule[0] = tone;
    for (i = 1; i < chord_run_data.rule_count + 1; i++)
    {
        chord_run_data.chord_rule[i] = (chord_rule[property].intervallic[i - 1] + tone) % 12;
    }

#ifdef CHORD_DEBUG
    printf("******************CHORD_RULE*********************\n");
    for (i = 0; i < chord_run_data.rule_count; i++)
    {
        printf("chord rules is %d\n", chord_run_data.chord_rule[i]);
    }
#endif

    for (check_over = 0; check_over < 3; check_over++)
    {

        loop_count = 0;
        string = chord_run_data.chord_result[check_over][6] + 1;
#ifdef CHORD_DEBUG
        printf("*******************GET_CHORD*****************\n");
#endif
        while (string < 6 && loop_count < 20)
        {

            for (i = 0; i < 12; i++)
            {

#ifdef SUPER_DEBUG_LEVEL
                printf("chord is %d run tone is %d,string is %d, fret is %d, right tone is %d\n", check_over, (base_tone[string] + i) % 12, string, i, chord_rules[count]);
#endif
                if (((base_tone[string] + i) % 12 == (chord_run_data.chord_rule[count]) % 12) && string < 6)
                {

                    if (i <= low_string_val[check_over] && i >= low_string_val[check_over] - chord_run_data.span

                    )
                    {
                        loop_count = 0;
#ifdef CHORD_DEBUG
                        printf("flag is 1 intervallic is %d string is %d fret is %d\n", count, string, i);
#endif
                        chord_run_data.chord_result[check_over][string] = i;
                        string++;
                    }
                }
            }
            loop_count++;
            count++;
            i = 0;
            if (count > chord_run_data.rule_count)
                count = 0;
#ifdef HIGH_DEBUG_LEVEL
            printf("property 1 count add is %d\n", count);
#endif
        }
        count = 0;
        loop_count = 0;
        string = chord_run_data.chord_result[check_over][6] + 1;
        while (string < 6 && loop_count < 100

        )
        {
            for (i = 0; i < 13; i++)
            {

#ifdef SUPER_DEBUG_LEVEL
                printf("chord is %d run tone is %d,string is %d, fret is %d, right tone is %d\n", check_over, (base_tone[string] + i) % 12, string, i, chord_rules[count]);
#endif

                if (((base_tone[string] + i) % 12 == (chord_run_data.chord_rule[count]) % 12) && string < 6)
                {

                    if (i <= chord_run_data.span + low_string_val[check_over] && i >= low_string_val[check_over])
                    {

                        loop_count = 0;
#ifdef CHORD_DEBUG
                        printf("chord is %d flag is 2 intervallic is %d string is %d fret is %d\n", check_over + 3, count, string, i);
#endif

                        chord_run_data.chord_result[check_over + 3][string] = i;
                        string++;
                    }
                }
            }
            loop_count++;
            count++;
            i = 0;
            if (count > chord_run_data.rule_count)
                count = 0;
#ifdef HIGH_DEBUG_LEVEL
            printf("property 2 count add is %d\n", count);
#endif
        }
    }

#ifdef CHORD_DEBUG
    for (uint8_t k = 0; k < 6; k++)
    {

        printf("*****************chord_generator**************\n");
        Graph_chord(chord_run_data.chord_result[k]);
        for (i = 0; i < 7; i++)
            printf("The Chord count is %d  string is %d  val is %d \n", k, i, chord_run_data.chord_result[k][i]);
    }
#endif
}

void Ergonomics_Check(uint8_t tone, uint8_t property)
{
    uint8_t check_flag = 0;
    uint8_t min = 0, max = 0, last_min = 0;
    uint8_t right_count = 0;
    for (uint8_t i = 0; i < 6; i++)
    {
        check_flag = 0;

        for (uint8_t string = chord_run_data.chord_result[i][6]; string < 6; string++)
        {

            right_count = 0;

            for (int8_t k = 0; k < chord_run_data.rule_count; k++)
            {

                if ((chord_run_data.chord_result[i][string] + base_tone[string]) % 12 ==
                    (chord_run_data.chord_rule[k]) % 12)
                {
                    right_count++;
#ifdef HIGH_DEBUG_LEVEL
                    printf("chord is %d string is %d run is %d right is %d val is %d\n", i, string,
                           (chord_run_data.chord_result[i][string] + base_tone[string]) % 12,
                           (chord_run_data.chord_rule[k]) % 12, (chord_run_data.chord_result[i][string]));
#endif
                }
            }
            if (right_count < 1)
            {
                check_flag = 1;
#ifdef CHORD_DEBUG
                printf("chord %d string %d give up!1\n", i, string);
#endif
            }
        }
        Get_MinMax_fret(chord_run_data.chord_result[i], &min, &max);
        if (max < 4 || chord_run_data.chord_result[i][6] > 1)
        {

            if (max - min > 3)
            {
                check_flag = 1;
#ifdef CHORD_DEBUG
                printf("chord %d give up!2\n", i);
#endif
            }
        }
        else
        {
            if (max - min > 2)
            {
                check_flag = 1;
#ifdef CHORD_DEBUG
                printf("chord %d give up!3\n", i);
#endif
            }
        }

        if (check_flag)
        {
            chord_run_data.chord_result[i][6] = 0xff;
        }
#ifdef CHORD_DEBUG
        if (chord_run_data.chord_result[i][6] != 0xff)
        {
            printf("*****************Ergonomics_Check*****************\n");
            Graph_chord(chord_run_data.chord_result[i]);
            for (uint8_t j = 0; j < 7; j++)

                printf("The Chord count is %d  string is %d  val is %d \n", i, j, chord_run_data.chord_result[i][j]);
        }

#endif
    }
}

void Chord_Filter(uint8_t *chord)
{


}

BG_ERR BG_chord_Auto_Chord(uint8_t *chord, uint8_t tone, uint8_t property)
{

    chord_generator(chord, tone, property);

    Ergonomics_Check(tone, property);

    return SUCCESS;
}

uint8_t BG_chord_status(void)
{

    return chord_run_data.enable;
}

uint8_t BG_chord_getmode(void)
{

    return chord_run_data.mode;
}