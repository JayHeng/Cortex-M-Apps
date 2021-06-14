/*
 * Copyright (c) 2021 NXP
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
//! @brief ��ȡGPIO�ܽ������ƽ
extern uint32_t read_autobaud_pin(void);

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void autobaud_init(void)
{
    // ȷ����ƽΪ�ߣ�����̬��
    while (read_autobaud_pin() != 1);
}

void autobaud_deinit(void)
{

}

bool autobaud_get_rate(uint32_t *rate)
{
    // ������ƽΪ�ͣ��ǿ���̬��ʱ�ſ�ʼʶ��
    uint32_t currentEdge = read_autobaud_pin();
    if (currentEdge != 1)
    {
        uint64_t ticks = microseconds_get_ticks();
        uint32_t transitionCount = 1;
        // 0x5A �ֽڼ��ʱ�����
        uint64_t firstByteTotalTicks = ticks;
        uint64_t secondByteTotalTicks;
        uint64_t lastToggleTicks = ticks;
        // ������½���֮�����ʱ��Ӧ����ֵ
        uint64_t ticksBetweenFailure = microseconds_convert_to_ticks(kMaximumTimeBetweenFallingEdges);
        uint32_t previousEdge = currentEdge;
        while (transitionCount < kFirstByteRequiredFallingEdges + kSecondByteRequiredFallingEdges)
        {
            // ��ȡ��ǰϵͳ����ֵ
            ticks = microseconds_get_ticks();
            // ����Ƿ��е�ƽ��ת
            currentEdge = read_autobaud_pin();
            if (currentEdge != previousEdge)
            {
                // ������ƽ��ת���½���ʱ
                if (previousEdge == 1)
                {
                    // ������μ�⵽���½���
                    transitionCount++;
                    // ��¼�����½��ط���ʱϵͳ����ֵ
                    lastToggleTicks = ticks;
                    // �õ� 0x5A �ֽڼ���ڼ��ڶ�Ӧ����ֵ
                    if (transitionCount == kFirstByteRequiredFallingEdges)
                    {
                        firstByteTotalTicks = ticks - firstByteTotalTicks;
                    }
                    // 0xA6 �ֽڼ��ʱ�����
                    else if (transitionCount == kFirstByteRequiredFallingEdges + 1)
                    {
                        secondByteTotalTicks = ticks;
                    }
                    // �õ� 0xA6 �ֽڼ���ڼ��ڶ�Ӧ����ֵ
                    else if (transitionCount == kFirstByteRequiredFallingEdges + kSecondByteRequiredFallingEdges)
                    {
                        secondByteTotalTicks = ticks - secondByteTotalTicks;
                    }
                }
                previousEdge = currentEdge;
            }
            // ��������½������ϴ��½���֮�������������ͷ��ʼ���
            if ((ticks - lastToggleTicks) > ticksBetweenFailure)
            {
                return false;
            }
        }

        // �����ʵ�ʼ�⵽�Ĳ�����ֵ
        uint32_t calculatedBaud =
            (microseconds_get_clock() * (kNumberOfBitsForFirstByteMeasured + kNumberOfBitsForSecondByteMeasured)) /
            (uint32_t)(firstByteTotalTicks + secondByteTotalTicks);
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

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////