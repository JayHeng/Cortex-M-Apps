/*
 * Copyright (c) 2013, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "autobaud.h"
#include "microseconds.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
enum _autobaud_counts
{
    //! 0x5A �ֽڶ�Ӧ���½��ظ���
    kFirstByteRequiredFallingEdges = 4,
    //! 0xA6 �ֽڶ�Ӧ���½��ظ���
    kSecondByteRequiredFallingEdges = 3,
    //! 0x5A �ֽڣ�����ʼλ��ֹͣλ����һ���½��ص����һ���½���֮���ʵ��bit��
    kNumberOfBitsForFirstByteMeasured = 8,
    //! 0xA6 �ֽڣ�����ʼλ��ֹͣλ����һ���½��ص����һ���½���֮���ʵ��bit��
    kNumberOfBitsForSecondByteMeasured = 7,
    //! �����½���֮����������ʱ(us)
    kMaximumTimeBetweenFallingEdges = 80000,
    //! ��ʵ�ʼ����Ĳ�����ֵ�����봦���Ա��ڸ��õ�����UARTģ��
    kAutobaudStepSize = 1200
};

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
//! @brief �ܽ��½�������ص�����
static void pin_transition_callback(void);
//! @brief ʹ��GPIO�ܽ��ж�
extern void enable_autobaud_pin_irq(pin_irq_callback_t func);
//! @brief �ر�GPIO�ܽ��ж�
extern void disable_autobaud_pin_irq(void);

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

//!< �Ѽ�⵽���½��ظ���
static uint32_t s_transitionCount;
//!< 0x5A �ֽڼ���ڼ��ڶ�Ӧ����ֵ
static uint64_t s_firstByteTotalTicks;
//!< 0xA6 �ֽڼ���ڼ��ڶ�Ӧ����ֵ
static uint64_t s_secondByteTotalTicks;
//!< ��һ���½��ط���ʱϵͳ����ֵ
static uint64_t s_lastToggleTicks;
//!< �½���֮�����ʱ��Ӧ����ֵ
static uint64_t s_ticksBetweenFailure;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void autobaud_init(void)
{
    s_transitionCount = 0;
    s_firstByteTotalTicks = 0;
    s_secondByteTotalTicks = 0;
    s_lastToggleTicks = 0;
    // ������½���֮�����ʱ��Ӧ����ֵ
    s_ticksBetweenFailure = microseconds_convert_to_ticks(kMaximumTimeBetweenFallingEdges);
    // ʹ��GPIO�ܽ��жϣ���ע���жϴ���ص�����
    enable_autobaud_pin_irq(pin_transition_callback);
}

void autobaud_deinit(void)
{
    // �ر�GPIO�ܽ��ж�
    disable_autobaud_pin_irq();
}

bool autobaud_get_rate(uint32_t *rate)
{
    if (s_transitionCount == (kFirstByteRequiredFallingEdges + kSecondByteRequiredFallingEdges))
    {
        // �����ʵ�ʼ�⵽�Ĳ�����ֵ
        uint32_t calculatedBaud =
            (microseconds_get_clock() * (kNumberOfBitsForFirstByteMeasured + kNumberOfBitsForSecondByteMeasured)) /
            (uint32_t)(s_firstByteTotalTicks + s_secondByteTotalTicks);

        // ��ʵ�ʼ����Ĳ�����ֵ�����봦��
        // ��ʽ��rounded = stepSize * (value/stepSize + .5)
        *rate = ((((calculatedBaud * 10) / kAutobaudStepSize) + 5) / 10) * kAutobaudStepSize;

        return true;
    }
    else
    {
        return false;
    }
}

void pin_transition_callback(void)
{
    // ��ȡ��ǰϵͳ����ֵ
    uint64_t ticks = microseconds_get_ticks();
    // ������μ�⵽���½���
    s_transitionCount++;

    // ��������½������ϴ��½���֮�������������ͷ��ʼ���
    uint64_t delta = ticks - s_lastToggleTicks;
    if (delta > s_ticksBetweenFailure)
    {
        s_transitionCount = 1;
    }

    switch (s_transitionCount)
    {
        case 1:
            // 0x5A �ֽڼ��ʱ�����
            s_firstByteTotalTicks = ticks;
            break;

        case kFirstByteRequiredFallingEdges:
            // �õ� 0x5A �ֽڼ���ڼ��ڶ�Ӧ����ֵ
            s_firstByteTotalTicks = ticks - s_firstByteTotalTicks;
            break;

        case (kFirstByteRequiredFallingEdges + 1):
            // 0xA6 �ֽڼ��ʱ�����
            s_secondByteTotalTicks = ticks;
            break;

        case (kFirstByteRequiredFallingEdges + kSecondByteRequiredFallingEdges):
            // �õ� 0xA6 �ֽڼ���ڼ��ڶ�Ӧ����ֵ
            s_secondByteTotalTicks = ticks - s_secondByteTotalTicks;
            // �ر�GPIO�ܽ��ж�
            disable_autobaud_pin_irq();
            break;
    }

    // ��¼�����½��ط���ʱϵͳ����ֵ
    s_lastToggleTicks = ticks;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
