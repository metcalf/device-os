/**
 * @file    FP.cpp
 * @brief   Core Utility - Templated Function Pointer Class
 * @author  sam grove
 * @version 1.0
 * @see
 *
 * Copyright (c) 2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FP.h"
#include "MQTTClient.hpp"
#include <stdint.h>

template<class retT, class argT>
FP<retT, argT>::FP()
{
    obj_callback = 0;
    c_callback = 0;
}

template<class retT, class argT>
bool FP<retT, argT>::attached()
{
    return obj_callback || c_callback;
}


template<class retT, class argT>
void FP<retT, argT>::detach()
{
    obj_callback = 0;
    c_callback = 0;
}


template<class retT, class argT>
void FP<retT, argT>::attach(retT (*function)(argT))
{
    c_callback = function;
}

template<class retT, class argT>
retT FP<retT, argT>::operator()(argT arg) const
{
    if( 0 != c_callback )
    {
        return obj_callback ? (obj_callback->*method_callback)(arg) : (*c_callback)(arg);
    }
    return (retT)0;
}

template class FP<void,MQTT::MessageData*>;
template class FP<void,void*>;
