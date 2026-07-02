// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.
// Portions of the image math are derived from open-display-transform by Jed Smith.

#ifndef ODT_N6_COLOR_PARAMS_H
#define ODT_N6_COLOR_PARAMS_H

typedef struct {
  int transferFunction;

  int enablePurity;
  int enableChromaValue;
  int enableVibrance;
  int enableHueShift;
  int enableCrossTalk;

  float outputMix;
  float purityMix;
  float chromaValueMix;
  float vibranceMix;
  float hueShiftMix;
  float crossTalkMix;

  float purityRed;
  float purityRedStrength;
  float purityGreen;
  float purityGreenStrength;
  float purityBlue;
  float purityBlueStrength;
  float purityCyan;
  float purityCyanStrength;
  float purityMagenta;
  float purityMagentaStrength;
  float purityYellow;
  float purityYellowStrength;

  float chromaValueYellow;
  float chromaValueRed;
  float chromaValueMagenta;
  float chromaValueBlue;
  float chromaValueCyan;
  float chromaValueGreen;
  float chromaValueHueStrengthRgb;
  float chromaValueHueStrengthCmy;
  float chromaValueChromaStrength;
  float chromaValueChromaLimit;
  int chromaValueZoneEnable;
  float chromaValueZoneRange;
  int chromaValueZoneTarget;

  float vibranceGlobal;
  float vibranceYellow;
  float vibranceRed;
  float vibranceMagenta;
  float vibranceBlue;
  float vibranceCyan;
  float vibranceGreen;
  float vibranceCustom;
  float vibranceCustomHue;
  float vibranceCustomWidth;
  float vibranceHueLinear;
  int vibranceZoneEnable;
  float vibranceZoneRange;
  int vibranceZoneTarget;

  float hueShiftYellow;
  float hueShiftRed;
  float hueShiftMagenta;
  float hueShiftBlue;
  float hueShiftCyan;
  float hueShiftGreen;
  float hueShiftCustom;
  float hueShiftCustomHue;
  float hueShiftCustomWidth;
  float hueShiftStrength;
  float hueShiftChromaLimit;
  int hueShiftZoneEnable;
  float hueShiftZoneRange;
  int hueShiftZoneTarget;

  float crossTalkYellowPower;
  float crossTalkYellowShift0;
  float crossTalkYellowShift1;
  float crossTalkYellowScale;
  float crossTalkRedPower;
  float crossTalkRedShift0;
  float crossTalkRedShift1;
  float crossTalkRedScale;
  float crossTalkMagentaPower;
  float crossTalkMagentaShift0;
  float crossTalkMagentaShift1;
  float crossTalkMagentaScale;
  float crossTalkBluePower;
  float crossTalkBlueShift0;
  float crossTalkBlueShift1;
  float crossTalkBlueScale;
  float crossTalkCyanPower;
  float crossTalkCyanShift0;
  float crossTalkCyanShift1;
  float crossTalkCyanScale;
  float crossTalkGreenPower;
  float crossTalkGreenShift0;
  float crossTalkGreenShift1;
  float crossTalkGreenScale;
  float crossTalkCyanCenter;
  float crossTalkMagentaCenter;
  float crossTalkYellowCenter;
} ODTN6ColorParams;

#endif // ODT_N6_COLOR_PARAMS_H
