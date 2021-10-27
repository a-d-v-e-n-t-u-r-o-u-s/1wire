/*!
 * \file
 * \brief 1-WIRE implementation file
 * \author Dawid Babula
 * \email dbabula@adventurous.pl
 *
 * \par Copyright (C) Dawid Babula, 2019
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "1wire.h"
#include "hardware.h"
#include <stddef.h>
#include <limits.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define MASTER_TX_RESET_PULSE_TIME          (500u)

/*
 * \todo (DB) setting this time to 45us can cause that sensor won't be discovered
 * because according to the documentation it has time till 60u, think about setting
 * this time to 60uS
 */
#define MASTER_WAIT_FOR_PRESENCE            (45u)
#define MASTER_WAIT_POST_PRESENCE           (470u)
#define MASTER_TX_SLOT_INIT_TIME            (5u)
#define MASTER_TX_HOLD_FOR_SAMPLING         (80u)
#define MASTER_SLOT_RECOVERY_TIME           (2u)
#define MASTER_RX_SLOT_INIT_TIME            (2u)
#define MASTER_RX_WAIT_TO_SAMPLE            (15u)

static void send_bit(bool bit)
{
    cli();

    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_OUTPUT_PUSH_PULL);
    _delay_us(MASTER_TX_SLOT_INIT_TIME);

    if(bit)
    {
        GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_INPUT_FLOATING);
    }

    _delay_us(MASTER_TX_HOLD_FOR_SAMPLING);
    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_INPUT_FLOATING);
    sei();
}

static bool read_bit(void)
{
    bool ret = false;
    cli();
    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_OUTPUT_PUSH_PULL);
    _delay_us(MASTER_RX_SLOT_INIT_TIME);
    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_INPUT_FLOATING);
    _delay_us(MASTER_RX_WAIT_TO_SAMPLE);

    ret = GPIO_read_pin(GPIO_CHANNEL_1WIRE);
    sei();
    return ret;
}

bool WIRE_reset(void)
{
    bool ret = false;

    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_OUTPUT_PUSH_PULL);
    _delay_us(MASTER_TX_RESET_PULSE_TIME);

    GPIO_config_pin(GPIO_CHANNEL_1WIRE, GPIO_INPUT_FLOATING);
    _delay_us(MASTER_WAIT_FOR_PRESENCE);

    ret = !GPIO_read_pin(GPIO_CHANNEL_1WIRE);

    _delay_us(MASTER_WAIT_POST_PRESENCE);

    return ret;
}

void WIRE_send_byte(uint8_t byte)
{
    uint8_t tmp = byte;

    for(uint8_t i = 0U; i < CHAR_BIT; i++)
    {
        const bool bit = ((tmp & 0x01U) == 0x01);
        send_bit(bit);
        tmp >>= 1U;
        _delay_us(MASTER_SLOT_RECOVERY_TIME);
    }
}

uint8_t WIRE_read_byte(void)
{
    uint8_t ret = 0U;

    for(uint8_t i = 0U; i < CHAR_BIT ; i++)
    {
        const bool bit = read_bit();
        if(bit)
        {
            ret |= (1U << i);
        }
        _delay_us(MASTER_SLOT_RECOVERY_TIME);
    }
    return ret;
}

void WIRE_configure(void)
{
    /* is going to be implemented later */
}
