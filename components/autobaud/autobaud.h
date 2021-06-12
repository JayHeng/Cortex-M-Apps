/*
 * Copyright (c) 2013, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _autobaud_h
#define _autobaud_h

#include <stdint.h>
#include <stdbool.h>

//! @addtogroup autobaud
//! @{

/*******************************************************************************
 * Definitions
 ******************************************************************************/

//! @brief �ܽ��½�������ص�����ԭ��.
typedef void (*pin_irq_callback_t)(void);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

//! @name Autobaud
//@{

//! @brief Initializes autobaud detection
//! @brief ��ʼ��������ʶ��
void autobaud_init(void);

//! @brief If completed sets the rate after calculation as 9600, 19200, etc.
//         If not yet completed returns failure
//! @brief ��Ⲩ����ʶ���Ƿ�����ɣ�����ȡ������ֵ
bool autobaud_get_rate(uint32_t *rate);

//! @brief De-initializes autobaud detection
//! @brief �رղ�����ʶ��
void autobaud_deinit(void);

//@}

#if defined(__cplusplus)
}
#endif

//! @}

#endif // _autobaud_h
