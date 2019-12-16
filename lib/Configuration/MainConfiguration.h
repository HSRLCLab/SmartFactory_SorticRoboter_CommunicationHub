/**
 * @file MainConfiguration.h
 * @author Philip Zellweger (philip.zellweger@hsr.ch)
 * @brief Configurations for the sortic communication hub
 * @version 1.1
 * @date 2019-12-16
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef MAINCONFIGURATION_H__
#define MAINCONFIGURATION_H__

#include <deque>
#include <memory>
#include "MessageTranslation.h"

#define Master
#define I2CMASTERADDRESP 33                 ///< I2C adress of master
#define I2CSLAVEADDRUNO 7                   ///< I2C adress of slave

#define DEFAULT_HOSTNAME "Sortic"           ///< Hostname
#define TIME_BETWEEN_PUBLISH 300            ///< Time between publish
#define TIME_BETWEEN_SUBSCRIBE 5000         ///< Time between subscribe

#endif // MAINCONFIGURATION_H__