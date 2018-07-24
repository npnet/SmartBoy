/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */

#ifndef _USER_INTERFACE_KEY_H_
#define _USER_INTERFACE_KEY_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/


//#ifdef __cplusplus
extern "C"
{
//#endif // __cplusplus

#include "keydef.h"
#define BTN_IR_DIGIT_1    2       
#define BTN_IR_DIGIT_2    3         
#define BTN_IR_DIGIT_3    4         
#define BTN_IR_DIGIT_4    5         
#define BTN_IR_DIGIT_5    6         
#define BTN_IR_DIGIT_6    7       
#define BTN_IR_DIGIT_7    8         
#define BTN_IR_DIGIT_8    9         
#define BTN_IR_DIGIT_9    10
#define BTN_IR_DIGIT_0    11
#define BTN_IR_POWER      116    //map power key
#define BTN_IR_DISPLAY    46     //map source key
#define BTN_IR_EJECT      32     //map request key
#define BTN_IR_CLEAR      14     //map BT key
#define BTN_IR_GOTO       45     //map surround key
#define BTN_IR_OSC        18
#define BTN_IR_HOME       172   
#define BTN_IR_RETURN     158   //map mute key
#define BTN_IR_UP         103
#define BTN_IR_DOWN       108
#define BTN_IR_LEFT       105
#define BTN_IR_RIGHT      106
#define BTN_IR_ENTER      28
#define BTN_IR_TOPMENU    47
#define BTN_IR_POPTITLE   48
#define BTN_IR_PROGRAM    20
#define BTN_IR_BOOKMARK   21
#define BTN_IR_DIGEST     22
#define BTN_IR_ZOOM       23
#define BTN_IR_PREV       165
#define BTN_IR_NEXT       163
#define BTN_IR_SB         25
#define BTN_IR_SF         30
#define BTN_IR_FB         168
#define BTN_IR_FF         208
#define BTN_IR_PAUSE      201
#define BTN_IR_PLAY       200
#define BTN_IR_STOP       166
#define BTN_IR_REPEAT_ALL 36
#define BTN_IR_REPEAT_AB  37
#define BTN_IR_2ND_AUDIO  50
#define BTN_IR_SUBTITLE   38
#define BTN_IR_AUDIO      17
#define BTN_IR_ANGLE      44
#define BTN_IR_PIP        49


typedef struct{
int16 IR_Keycode;
uint16 SM_Msg_Type;
char* SM_MSG_NAME;
}IR_SM_MAP;

//#ifdef __cplusplus
} // extern "C"
//#endif // __cplusplus

#endif /* _USER_INTERFACE_KEY_H_ */

