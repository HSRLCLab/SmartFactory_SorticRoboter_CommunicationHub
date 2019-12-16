/**
 * @file MainConfiguration.h
 * @author Philip Zellweger (philip.zellweger@hsr.ch)
 * @brief 
 * @version 0.1
 * @date 2019-11-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef MAINCONFIGURATION_H
#define MAINCONFIGURATION_H

#include <deque>
#include <memory>
#include "MessageTranslation.h"

#define Master
#define I2CMASTERADDRESP 33
#define I2CSLAVEADDRUNO 7
#define I2C_TOPIC_SIZE 3

#define DEFAULT_HOSTNAME "Sortic"
#define SORTIC_WAITFOR_BOX_SECONDS 5
#define TIME_BETWEEN_PUBLISH 300
#define TIME_BETWEEN_SUBSCRIBE 5000
#define SORTIC_ITERATION_VACKS_SECONDS 5



#endif