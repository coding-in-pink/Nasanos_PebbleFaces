#pragma once
#include "pebble.h"

static const GPathInfo TICK_POINTS = {
  4, (GPoint []) {
      {-3, -6},
      {3, -6},
      {3, 8},
      {-3, 8}
  }
};
static const GPathInfo MIN_TICK_POINTS = {
  4, (GPoint []) {
      {-3, -6},
      {3, -6},
      {2, 3},
      {-2, 3}
  }
};

static const GPathInfo MINUTE_HAND_POINTS = {
  8, (GPoint []){
    {-2, -2},
    {2, -2},
    {2, -24},
    {4, -24},
    {0, -70},
    {0, -70},
    {-4, -24},
    {-2, -24}
  }
};
static const GPathInfo HOUR_HAND_POINTS = {
  8, (GPoint []){
    {-2, -2},
    {2, -2},
    {2, -12},
    {4, -12},
    {2, -54},
    {-2, -54},
    {-4, -12},
    {-2, -12}
  }
};

static const GPathInfo L_FOOT_POINTS = {
  8, (GPoint []){
    {-2, -2},
    {2, -2},
    {2, -24},
    {4, -24},
    {2, -70},
    {-2, -70},
    {-4, -24},
    {-2, -24}
  }
};
static const GPathInfo R_FOOT_POINTS = {
  8, (GPoint []){
    {-2, -2},
    {2, -2},
    {2, -12},
    {4, -12},
    {3, -54},
    {-3, -54},
    {-4, -12},
    {-2, -12}
  }
};