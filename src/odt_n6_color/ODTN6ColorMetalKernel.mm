// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.
// Portions of the image math are derived from open-display-transform by Jed Smith.

#include "ODTN6ColorParams.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <cstdio>
#include <mutex>
#include <unordered_map>

static const char *kMetalSource = R"METAL(
#include <metal_stdlib>
using namespace metal;

struct ODTN6ColorParams {
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
};

float sdivf(float a, float b) {
  return abs(b) < 0.0001f ? 0.0f : a / b;
}

float3 sdivf3f(float3 a, float b) {
  return float3(sdivf(a.x, b), sdivf(a.y, b), sdivf(a.z, b));
}

float spowf(float a, float b) {
  return a <= 0.0f ? a : pow(a, b);
}

float extractRange(float e0, float e1, float x) {
  return clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
}

float extractWindow(float e0, float e1, float e2, float e3, float x) {
  return x < e1 ? extractRange(e0, e1, x) : extractRange(e3, e2, x);
}

float extractHueAngle(float h, float o, float w, int sm) {
  float hc = extractWindow(2.0f - w, 2.0f, 2.0f, 2.0f + w,
                           fmod(h + o, 6.0f));
  if (sm == 1) {
    hc = hc * hc * (3.0f - 2.0f * hc);
  }
  return hc;
}

float calcHue(float3 rgb) {
  float mx = max(rgb.x, max(rgb.y, rgb.z));
  float mn = min(rgb.x, min(rgb.y, rgb.z));
  float ch = mx - mn;
  if (ch == 0.0f) return 0.0f;
  if (mx == rgb.x) return fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
  if (mx == rgb.y) return (rgb.z - rgb.x) / ch + 2.0f;
  return (rgb.x - rgb.y) / ch + 4.0f;
}

float calcChroma(float3 rgb) {
  float mx = max(rgb.x, max(rgb.y, rgb.z));
  float mn = min(rgb.x, min(rgb.y, rgb.z));
  return sdivf(mx - mn, mx);
}

float3 zoneExtract(float3 in, float3 rgb, float zp, int zr) {
  float n = max(1.0e-12f, max(rgb.x, max(rgb.y, rgb.z)));
  float fl = 0.01f;
  float zpow = pow(2.0f, -zp + 1.0f);
  float toe = (n * n / (n + fl));
  float f = pow((toe / (toe + 1.0f)) / n, zpow);
  if (zr == 1) {
    f = 1.0f - pow((n / (n + 1.0f)) / n, zpow);
  }
  return in * (1.0f - f) + rgb * f;
}

float oetfDavinciIntermediate(float x, int inv) {
  if (inv == 1) {
    return x <= 0.02740668f ? x / 10.44426855f
                            : exp2(x / 0.07329248f - 7.0f) - 0.0075f;
  }
  return x <= 0.00262409f
             ? x * 10.44426855f
             : (log2(x + 0.0075f) + 7.0f) * 0.07329248f;
}

float oetfAcescct(float x, int inv) {
  if (inv == 1) {
    return x <= 0.155251141552511f
               ? (x - 0.0729055341958355f) / 10.5402377416545f
               : exp2(x * 17.52f - 9.72f);
  }
  return x <= 0.0078125f ? 10.5402377416545f * x + 0.0729055341958355f
                         : (log2(x) + 9.72f) / 17.52f;
}

float oetfArriLogC3(float x, int inv) {
  if (inv == 1) {
    return x < 5.367655f * 0.010591f + 0.092809f
               ? (x - 0.092809f) / 5.367655f
               : (pow(10.0f, (x - 0.385537f) / 0.247190f) - 0.052272f) /
                     5.555556f;
  }
  return x < 0.010591f
             ? 5.367655f * x + 0.092809f
             : 0.247190f * log10(5.555556f * x + 0.052272f) + 0.385537f;
}

float oetfArriLogC4(float x, int inv) {
  if (inv == 1) {
    return x < -0.7774983977293537f
               ? x * 0.3033266726886969f - 0.7774983977293537f
               : (exp2(14.0f * (x - 0.09286412512218964f) /
                           0.9071358748778103f +
                       6.0f) -
                  64.0f) /
                     2231.8263090676883f;
  }
  return x < -0.7774983977293537f
             ? (x - -0.7774983977293537f) / 0.3033266726886969f
             : (log2(2231.8263090676883f * x + 64.0f) - 6.0f) / 14.0f *
                       0.9071358748778103f +
                   0.09286412512218964f;
}

float oetfRedLog3G10(float x, int inv) {
  float a = 0.224282f;
  float b = 155.975327f;
  float c = 0.01f;
  float g = 15.1927f;
  if (inv == 1) {
    return x < 0.0f ? (x / g) - c : (pow(10.0f, x / a) - 1.0f) / b - c;
  }
  return x < -c ? (x + c) * g : a * log10((x + c) * b + 1.0f);
}

float3 linearize(float3 rgb, int tf, int inv) {
  if (tf == 1) {
    return float3(oetfDavinciIntermediate(rgb.x, !inv),
                  oetfDavinciIntermediate(rgb.y, !inv),
                  oetfDavinciIntermediate(rgb.z, !inv));
  }
  if (tf == 2) {
    return float3(oetfAcescct(rgb.x, !inv), oetfAcescct(rgb.y, !inv),
                  oetfAcescct(rgb.z, !inv));
  }
  if (tf == 3) {
    return float3(oetfArriLogC3(rgb.x, !inv), oetfArriLogC3(rgb.y, !inv),
                  oetfArriLogC3(rgb.z, !inv));
  }
  if (tf == 4) {
    return float3(oetfArriLogC4(rgb.x, !inv), oetfArriLogC4(rgb.y, !inv),
                  oetfArriLogC4(rgb.z, !inv));
  }
  if (tf == 5) {
    return float3(oetfRedLog3G10(rgb.x, !inv),
                  oetfRedLog3G10(rgb.y, !inv),
                  oetfRedLog3G10(rgb.z, !inv));
  }
  return rgb;
}

float3 n6ChromaValue(float3 rgb, float my, float mr, float mm, float mb,
                     float mc, float mg, float hsRgb, float hsCmy, float chs,
                     float chl, int ze, float zp, int zr) {
  float3 mp = float3(pow(2.0f, mr), pow(2.0f, mg), pow(2.0f, mb));
  float3 ms = float3(pow(2.0f, mc), pow(2.0f, mm), pow(2.0f, my));
  float3 in = rgb;
  float hue = calcHue(rgb);
  float ch = calcChroma(rgb);
  float chStr = spowf(min(1.0f, ch), sdivf(1.0f, chs));
  chStr = chStr * spowf(1.0f - chStr, chl);

  float3 hp = float3(extractHueAngle(hue, 2.0f, 1.0f, 0),
                     extractHueAngle(hue, 6.0f, 1.0f, 0),
                     extractHueAngle(hue, 4.0f, 1.0f, 0));
  float3 hs = float3(extractHueAngle(hue, 5.0f, 1.0f, 0),
                     extractHueAngle(hue, 3.0f, 1.0f, 0),
                     extractHueAngle(hue, 1.0f, 1.0f, 0));
  float3 pp = float3(min(hsRgb, hsRgb / mp.x), min(hsRgb, hsRgb / mp.y),
                     min(hsRgb, hsRgb / mp.z));
  float3 ps = float3(min(hsCmy, hsCmy / ms.x), min(hsCmy, hsCmy / ms.y),
                     min(hsCmy, hsCmy / ms.z));
  float3 hpW = float3(1.0f - spowf(1.0f - hp.x, pp.x),
                      1.0f - spowf(1.0f - hp.y, pp.y),
                      1.0f - spowf(1.0f - hp.z, pp.z));
  float3 hsW = float3(1.0f - spowf(1.0f - hs.x, ps.x),
                      1.0f - spowf(1.0f - hs.y, ps.y),
                      1.0f - spowf(1.0f - hs.z, ps.z));
  float mf = ((1.0f - hpW.x) + mp.x * hpW.x) *
             ((1.0f - hpW.y) + mp.y * hpW.y) *
             ((1.0f - hpW.z) + mp.z * hpW.z) *
             ((1.0f - hsW.x) + ms.x * hsW.x) *
             ((1.0f - hsW.y) + ms.y * hsW.y) *
             ((1.0f - hsW.z) + ms.z * hsW.z);
  float mfLim = max(1.0f, mf);
  float chf = sdivf(chStr, (chStr * (1.0f - mfLim) + mfLim));
  rgb = rgb * mf * chf + rgb * (1.0f - chf);
  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

float3 n6Vibrance(float3 rgb, float mgl, float my, float mr, float mm,
                  float mb, float mc, float mg, float mcu, float cuh,
                  float cuw, float hl, int ze, float zp, int zr) {
  float3 in = rgb;
  float pBase = 3.0f;
  float mglStr = pow(pBase, mgl);
  float3 pp = float3(pow(pBase, mr) * mglStr, pow(pBase, mg) * mglStr,
                     pow(pBase, mb) * mglStr);
  float3 ps = float3(pow(pBase, mc) * mglStr, pow(pBase, mm) * mglStr,
                     pow(pBase, my) * pow(pBase / 1.5f, mgl));
  float pc = pow(pBase, mcu);
  float n = max(rgb.x, max(rgb.y, rgb.z));
  float3 r = n == 0.0f ? float3(0.0f) : rgb / n;
  r = max(r, float3(-1.0f));
  float h = calcHue(r);
  float c = 1.0f - min(r.x, min(r.y, r.z));
  float3 rHl = r;
  float3 hp = float3(extractHueAngle(h, 2.0f, 1.0f, 0),
                     extractHueAngle(h, 6.0f, 1.0f, 0),
                     extractHueAngle(h, 4.0f, 1.0f, 0)) *
              c;
  float3 hs = float3(extractHueAngle(h, 5.0f, 1.0f, 0),
                     extractHueAngle(h, 3.0f, 1.0f, 0),
                     extractHueAngle(h, 1.0f, 1.0f, 0)) *
              c;
  float hc = extractHueAngle(h, cuh / 60.0f, cuw, 0) * c;

  r.x = r.x < 0.0f ? r.x
                   : pow(r.x, pp.x) * hp.x + pow(r.x, pp.y) * hp.y +
                         pow(r.x, pp.z) * hp.z +
                         r.x * (1.0f - (hp.x + hp.y + hp.z));
  r.y = r.y < 0.0f ? r.y
                   : pow(r.y, pp.x) * hp.x + pow(r.y, pp.y) * hp.y +
                         pow(r.y, pp.z) * hp.z +
                         r.y * (1.0f - (hp.x + hp.y + hp.z));
  r.z = r.z < 0.0f ? r.z
                   : pow(r.z, pp.x) * hp.x + pow(r.z, pp.y) * hp.y +
                         pow(r.z, pp.z) * hp.z +
                         r.z * (1.0f - (hp.x + hp.y + hp.z));

  r.x = r.x < 0.0f ? r.x
                   : pow(r.x, ps.x) * hs.x + pow(r.x, ps.y) * hs.y +
                         pow(r.x, ps.z) * hs.z +
                         r.x * (1.0f - (hs.x + hs.y + hs.z));
  r.y = r.y < 0.0f ? r.y
                   : pow(r.y, ps.x) * hs.x + pow(r.y, ps.y) * hs.y +
                         pow(r.y, ps.z) * hs.z +
                         r.y * (1.0f - (hs.x + hs.y + hs.z));
  r.z = r.z < 0.0f ? r.z
                   : pow(r.z, ps.x) * hs.x + pow(r.z, ps.y) * hs.y +
                         pow(r.z, ps.z) * hs.z +
                         r.z * (1.0f - (hs.x + hs.y + hs.z));

  r.x = r.x < 0.0f ? r.x : pow(r.x, pc) * hc + r.x * (1.0f - hc);
  r.y = r.y < 0.0f ? r.y : pow(r.y, pc) * hc + r.y * (1.0f - hc);
  r.z = r.z < 0.0f ? r.z : pow(r.z, pc) * hc + r.z * (1.0f - hc);

  float m = 1.1f;
  float vf = c == 0.0f || c > 1.0f
                 ? 1.0f
                 : ((1.0f - pow(1.0f - c, pp.x)) * hp.x +
                    (1.0f - pow(1.0f - c, pp.y)) * hp.y +
                    (1.0f - pow(1.0f - c, pp.z)) * hp.z +
                    c * (1.0f - (hp.x + hp.y + hp.z))) /
                       c;
  rHl = float3(m * (1.0f - vf)) + rHl * vf;
  m = 1.0f;
  vf = c == 0.0f || c > 1.0f
           ? 1.0f
           : ((1.0f - pow(1.0f - c, ps.x)) * hs.x +
              (1.0f - pow(1.0f - c, ps.y)) * hs.y +
              (1.0f - pow(1.0f - c, ps.z)) * hs.z +
              c * (1.0f - (hs.x + hs.y + hs.z))) /
                 c;
  rHl = float3(m * (1.0f - vf)) + rHl * vf;
  vf = c == 0.0f || c > 1.0f
           ? 1.0f
           : ((1.0f - pow(1.0f - c, pc)) * hc + c * (1.0f - hc)) / c;
  rHl = float3(m * (1.0f - vf)) + rHl * vf;

  r = r * (1.0f - hl) + rHl * hl;
  rgb = r * n;
  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

float3 n6HueShift(float3 rgb, float sy, float sr, float sm, float sb,
                  float sc, float sg, float scu, float cuh, float cuw,
                  float str, float chl, int ze, float zp, int zr) {
  float3 in = rgb;
  float n = max(rgb.x, max(rgb.y, rgb.z));
  float3 r = n == 0.0f ? float3(0.0f) : rgb / n;
  float ch = min(r.x, min(r.y, r.z)) * (1.0f - str) + str;
  ch = ch == 0.0f
           ? 0.0f
           : min(1.0f, 1.0f - min(r.x / ch, min(r.y / ch, r.z / ch)));
  float f0 = chl < 0.5f ? max(0.0f, 0.5f - chl) * 2.0f
                        : 1.0f / max(1.0e-3f, (1.0f - chl) * 2.0f);
  ch = chl < 0.5f ? ch * f0 + ch * (1.0f - ch) * (1.0f - f0)
                  : ch * pow(max(0.0f, 1.0f - ch), f0);
  float hue = calcHue(r);
  float3 hp = float3(extractHueAngle(hue, 2.0f, 1.0f, 0),
                     extractHueAngle(hue, 6.0f, 1.0f, 0),
                     extractHueAngle(hue, 4.0f, 1.0f, 0)) *
              ch;
  float3 hs = float3(extractHueAngle(hue, 5.0f, 1.0f, 0),
                     extractHueAngle(hue, 3.0f, 1.0f, 0),
                     extractHueAngle(hue, 1.0f, 1.0f, 0)) *
              ch;

  r.x = r.x + sb * hp.z - sg * hp.y;
  r.y = r.y + sr * hp.x - sb * hp.z;
  r.z = r.z + sg * hp.y - sr * hp.x;
  r.x = r.x + sy * hs.z - sm * hs.y;
  r.y = r.y + sc * hs.x - sy * hs.z;
  r.z = r.z + sm * hs.y - sc * hs.x;

  float hc = extractHueAngle(hue, cuh / 60.0f, cuw, 0) * ch;
  float h = cuh / 60.0f;
  float s = scu * cuw;
  float sc0 = h < 3.0f ? s - s * min(1.0f, abs(h - 2.0f))
                       : s * min(1.0f, abs(h - 5.0f)) - s;
  float sc1 = h < 1.0f
                  ? s - s * min(1.0f, abs(h))
                  : h < 5.0f ? s * min(1.0f, abs(h - 3.0f)) - s
                              : s - s * min(1.0f, abs(h - 6.0f));
  float sc2 = h < 2.0f ? s * min(1.0f, abs(h - 1.0f)) - s
                       : s - s * min(1.0f, abs(h - 4.0f));
  r.x = r.x + hc * (sc2 - sc1);
  r.y = r.y + hc * (sc0 - sc2);
  r.z = r.z + hc * (sc1 - sc0);
  rgb = r * n;
  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

float powerWindow(float x, float c, float p0, float p1, float x0, float x1) {
  return x > x1
             ? 0.0f
             : x > c ? pow(1.0f - pow((x - c) / (x1 - c), p0), p1)
                     : x > x0 ? pow(1.0f - pow((c - x) / (c - x0), p0), p1)
                              : 0.0f;
}

float pivotedPowerCubic(float x, float m, float p) {
  m = 1.0f - abs(m);
  float a0 = p * (m - 1.0f);
  float b0 = (1.0f - m) * (p + 1.0f);
  return x <= 0.0f ? m * x
                   : x > 1.0f ? x : x * (pow(x, p) * (a0 * x + b0) + m);
}

float3 n6Purity(float3 rgb, constant ODTN6ColorParams &p) {
  float mx = max(rgb.x, max(rgb.y, rgb.z));
  float mn = min(rgb.x, min(rgb.y, rgb.z));
  float ch = mx - mn;
  float h = 0.0f;
  if (ch != 0.0f) {
    if (mx == rgb.x) h = fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
    else if (mx == rgb.y) h = (rgb.z - rgb.x) / ch + 2.0f;
    else h = (rgb.x - rgb.y) / ch + 4.0f;
  }
  ch = sdivf(ch, mx);
  float hr = powerWindow(fmod(h + 1.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float hg = powerWindow(fmod(h + 5.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float hb = powerWindow(fmod(h + 3.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float hc = powerWindow(fmod(h + 4.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float hm = powerWindow(fmod(h + 2.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float hy = powerWindow(fmod(h + 0.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f);
  float cr = sdivf(p.purityRed < 0.0f
                       ? pivotedPowerCubic(ch, p.purityRed, p.purityRedStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityRed,
                                                  p.purityRedStrength),
                   ch);
  float cg = sdivf(p.purityGreen < 0.0f
                       ? pivotedPowerCubic(ch, p.purityGreen,
                                           p.purityGreenStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityGreen,
                                                  p.purityGreenStrength),
                   ch);
  float cb = sdivf(p.purityBlue < 0.0f
                       ? pivotedPowerCubic(ch, p.purityBlue,
                                           p.purityBlueStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityBlue,
                                                  p.purityBlueStrength),
                   ch);
  float cc = sdivf(p.purityCyan < 0.0f
                       ? pivotedPowerCubic(ch, p.purityCyan,
                                           p.purityCyanStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityCyan,
                                                  p.purityCyanStrength),
                   ch);
  float cm = sdivf(p.purityMagenta < 0.0f
                       ? pivotedPowerCubic(ch, p.purityMagenta,
                                           p.purityMagentaStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityMagenta,
                                                  p.purityMagentaStrength),
                   ch);
  float cy = sdivf(p.purityYellow < 0.0f
                       ? pivotedPowerCubic(ch, p.purityYellow,
                                           p.purityYellowStrength)
                       : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityYellow,
                                                  p.purityYellowStrength),
                   ch);
  float chf = (hr * cr + hg * cg + hb * cb + (1.0f - (hr + hg + hb))) *
              (hc * cc + hm * cm + hy * cy + (1.0f - (hc + hm + hy)));
  float wsum = rgb.x * 0.3f + rgb.y * 0.6f + rgb.z * 0.1f;
  return float3(wsum) * (1.0f - chf) + rgb * chf;
}

float3 n6CrossTalk(float3 rgb, constant ODTN6ColorParams &p) {
  float mx = max(rgb.x, max(rgb.y, rgb.z));
  float mn = min(rgb.x, min(rgb.y, rgb.z));
  float ch = mx - mn;
  float h = 0.0f;
  if (ch != 0.0f) {
    if (mx == rgb.x) h = fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
    else if (mx == rgb.y) h = (rgb.z - rgb.x) / ch + 2.0f;
    else h = (rgb.x - rgb.y) / ch + 4.0f;
  }
  ch = clamp(sdivf(ch, mx), 0.0f, 4.0f);
  float hch = mn * 0.5f + mx * 0.5f;
  rgb = max(sdivf3f(rgb, hch), float3(-2.0f));
  float hr = powerWindow(fmod(h + 1.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f) *
             spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkRedPower));
  float hg = powerWindow(fmod(h + 5.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f) *
             spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkGreenPower));
  float hb = powerWindow(fmod(h + 3.0f, 6.0f), 1.0f, 1.5f, 1.5f, 0.0f, 2.0f) *
             spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkBluePower));
  float hc =
      powerWindow(fmod(h + 4.0f, 6.0f), 1.0f + p.crossTalkCyanCenter, 1.5f,
                  1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkCyanPower));
  float hm =
      powerWindow(fmod(h + 2.0f, 6.0f), 1.0f + p.crossTalkMagentaCenter, 1.5f,
                  1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkMagentaPower));
  float hy =
      powerWindow(fmod(h + 0.0f, 6.0f), 1.0f + p.crossTalkYellowCenter, 1.5f,
                  1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / max(1.0e-5f, p.crossTalkYellowPower));

  rgb.x = (p.crossTalkMagentaScale + 1.0f) *
              (rgb.x - max(0.0f, p.crossTalkMagentaShift0) -
               min(0.0f, p.crossTalkMagentaShift1)) *
              hm +
          (p.crossTalkYellowScale + 1.0f) *
              (rgb.x + min(0.0f, p.crossTalkYellowShift0) +
               max(0.0f, p.crossTalkYellowShift1)) *
              hy +
          rgb.x * (1.0f - (hm + hy));
  rgb.y = (p.crossTalkCyanScale + 1.0f) *
              (rgb.y - max(0.0f, p.crossTalkCyanShift0) -
               min(0.0f, p.crossTalkCyanShift1)) *
              hc +
          (p.crossTalkYellowScale + 1.0f) *
              (rgb.y - max(0.0f, p.crossTalkYellowShift0) -
               min(0.0f, p.crossTalkYellowShift1)) *
              hy +
          rgb.y * (1.0f - (hc + hy));
  rgb.z = (p.crossTalkCyanScale + 1.0f) *
              (rgb.z + min(0.0f, p.crossTalkCyanShift0) +
               max(0.0f, p.crossTalkCyanShift1)) *
              hc +
          (p.crossTalkMagentaScale + 1.0f) *
              (rgb.z + min(0.0f, p.crossTalkMagentaShift0) +
               max(0.0f, p.crossTalkMagentaShift1)) *
              hm +
          rgb.z * (1.0f - (hc + hm));

  rgb.x = (p.crossTalkRedScale + 1.0f) * rgb.x * hr +
          rgb.x * (1.0f + min(0.0f, p.crossTalkGreenShift0)) *
              (1.0f + max(0.0f, p.crossTalkGreenShift1)) * hg +
          rgb.x * (1.0f - max(0.0f, p.crossTalkBlueShift0)) *
              (1.0f - min(0.0f, p.crossTalkBlueShift1)) * hb +
          rgb.x * (1.0f - (hr + hg + hb));
  rgb.y = (p.crossTalkGreenScale + 1.0f) * rgb.y * hg +
          rgb.y * (1.0f - max(0.0f, p.crossTalkRedShift0)) *
              (1.0f - min(0.0f, p.crossTalkRedShift1)) * hr +
          rgb.y * (1.0f + min(0.0f, p.crossTalkBlueShift0)) *
              (1.0f + max(0.0f, p.crossTalkBlueShift1)) * hb +
          rgb.y * (1.0f - (hr + hg + hb));
  rgb.z = (p.crossTalkBlueScale + 1.0f) * rgb.z * hb +
          rgb.z * (1.0f + min(0.0f, p.crossTalkRedShift0)) *
              (1.0f + max(0.0f, p.crossTalkRedShift1)) * hr +
          rgb.z * (1.0f - max(0.0f, p.crossTalkGreenShift0)) *
              (1.0f - min(0.0f, p.crossTalkGreenShift1)) * hg +
          rgb.z * (1.0f - (hr + hg + hb));
  return rgb * hch;
}

bool nearZero(float v) { return abs(v) <= 1.0e-6f; }
bool nearValue(float v, float target) { return abs(v - target) <= 1.0e-6f; }

bool purityNeutral(constant ODTN6ColorParams &p) {
  return nearZero(p.purityRed) && nearZero(p.purityGreen) &&
         nearZero(p.purityBlue) && nearZero(p.purityCyan) &&
         nearZero(p.purityMagenta) && nearZero(p.purityYellow);
}

bool chromaValueNeutral(constant ODTN6ColorParams &p) {
  return nearZero(p.chromaValueYellow) && nearZero(p.chromaValueRed) &&
         nearZero(p.chromaValueMagenta) && nearZero(p.chromaValueBlue) &&
         nearZero(p.chromaValueCyan) && nearZero(p.chromaValueGreen);
}

bool vibranceNeutral(constant ODTN6ColorParams &p) {
  return nearZero(p.vibranceGlobal) && nearZero(p.vibranceYellow) &&
         nearZero(p.vibranceRed) && nearZero(p.vibranceMagenta) &&
         nearZero(p.vibranceBlue) && nearZero(p.vibranceCyan) &&
         nearZero(p.vibranceGreen) && nearZero(p.vibranceCustom);
}

bool hueShiftNeutral(constant ODTN6ColorParams &p) {
  return nearZero(p.hueShiftYellow) && nearZero(p.hueShiftRed) &&
         nearZero(p.hueShiftMagenta) && nearZero(p.hueShiftBlue) &&
         nearZero(p.hueShiftCyan) && nearZero(p.hueShiftGreen) &&
         nearZero(p.hueShiftCustom);
}

bool crossTalkNeutral(constant ODTN6ColorParams &p) {
  return nearValue(p.crossTalkYellowPower, 1.0f) &&
         nearZero(p.crossTalkYellowShift0) &&
         nearZero(p.crossTalkYellowShift1) &&
         nearZero(p.crossTalkYellowScale) &&
         nearValue(p.crossTalkRedPower, 1.0f) &&
         nearZero(p.crossTalkRedShift0) && nearZero(p.crossTalkRedShift1) &&
         nearZero(p.crossTalkRedScale) &&
         nearValue(p.crossTalkMagentaPower, 1.0f) &&
         nearZero(p.crossTalkMagentaShift0) &&
         nearZero(p.crossTalkMagentaShift1) &&
         nearZero(p.crossTalkMagentaScale) &&
         nearValue(p.crossTalkBluePower, 1.0f) &&
         nearZero(p.crossTalkBlueShift0) && nearZero(p.crossTalkBlueShift1) &&
         nearZero(p.crossTalkBlueScale) &&
         nearValue(p.crossTalkCyanPower, 1.0f) &&
         nearZero(p.crossTalkCyanShift0) && nearZero(p.crossTalkCyanShift1) &&
         nearZero(p.crossTalkCyanScale) &&
         nearValue(p.crossTalkGreenPower, 1.0f) &&
         nearZero(p.crossTalkGreenShift0) &&
         nearZero(p.crossTalkGreenShift1) &&
         nearZero(p.crossTalkGreenScale) &&
         nearValue(p.crossTalkCyanCenter, 0.25f) &&
         nearZero(p.crossTalkMagentaCenter) &&
         nearValue(p.crossTalkYellowCenter, -0.25f);
}

bool hasActiveN6ColorEffect(constant ODTN6ColorParams &p) {
  if (nearZero(p.outputMix)) return false;
  return (p.enablePurity != 0 && !nearZero(p.purityMix) && !purityNeutral(p)) ||
         (p.enableChromaValue != 0 && !nearZero(p.chromaValueMix) &&
          !chromaValueNeutral(p)) ||
         (p.enableVibrance != 0 && !nearZero(p.vibranceMix) &&
          !vibranceNeutral(p)) ||
         (p.enableHueShift != 0 && !nearZero(p.hueShiftMix) &&
          !hueShiftNeutral(p)) ||
         (p.enableCrossTalk != 0 && !nearZero(p.crossTalkMix) &&
          !crossTalkNeutral(p));
}

float3 applyN6Color(float3 src, constant ODTN6ColorParams &p) {
  if (!hasActiveN6ColorEffect(p)) return src;
  float3 combined = src;
  if (p.enablePurity != 0 && !nearZero(p.purityMix) && !purityNeutral(p)) {
    float3 branch = n6Purity(src, p);
    combined += (branch - src) * p.purityMix;
  }
  if (p.enableChromaValue != 0 && !nearZero(p.chromaValueMix) &&
      !chromaValueNeutral(p)) {
    float3 branch = linearize(src, p.transferFunction, 0);
    branch = n6ChromaValue(branch, p.chromaValueYellow, p.chromaValueRed,
                           p.chromaValueMagenta, p.chromaValueBlue,
                           p.chromaValueCyan, p.chromaValueGreen,
                           p.chromaValueHueStrengthRgb,
                           p.chromaValueHueStrengthCmy,
                           p.chromaValueChromaStrength,
                           p.chromaValueChromaLimit,
                           p.chromaValueZoneEnable, p.chromaValueZoneRange,
                           p.chromaValueZoneTarget);
    branch = linearize(branch, p.transferFunction, 1);
    combined += (branch - src) * p.chromaValueMix;
  }
  if (p.enableVibrance != 0 && !nearZero(p.vibranceMix) &&
      !vibranceNeutral(p)) {
    float3 branch = linearize(src, p.transferFunction, 0);
    branch = n6Vibrance(branch, p.vibranceGlobal, p.vibranceYellow,
                        p.vibranceRed, p.vibranceMagenta, p.vibranceBlue,
                        p.vibranceCyan, p.vibranceGreen, p.vibranceCustom,
                        p.vibranceCustomHue, p.vibranceCustomWidth,
                        p.vibranceHueLinear, p.vibranceZoneEnable,
                        p.vibranceZoneRange, p.vibranceZoneTarget);
    branch = linearize(branch, p.transferFunction, 1);
    combined += (branch - src) * p.vibranceMix;
  }
  if (p.enableHueShift != 0 && !nearZero(p.hueShiftMix) &&
      !hueShiftNeutral(p)) {
    float3 branch = linearize(src, p.transferFunction, 0);
    branch = n6HueShift(branch, p.hueShiftYellow, p.hueShiftRed,
                        p.hueShiftMagenta, p.hueShiftBlue, p.hueShiftCyan,
                        p.hueShiftGreen, p.hueShiftCustom,
                        p.hueShiftCustomHue, p.hueShiftCustomWidth,
                        p.hueShiftStrength, p.hueShiftChromaLimit,
                        p.hueShiftZoneEnable, p.hueShiftZoneRange,
                        p.hueShiftZoneTarget);
    branch = linearize(branch, p.transferFunction, 1);
    combined += (branch - src) * p.hueShiftMix;
  }
  if (p.enableCrossTalk != 0 && !nearZero(p.crossTalkMix) &&
      !crossTalkNeutral(p)) {
    float3 branch = n6CrossTalk(src, p);
    combined += (branch - src) * p.crossTalkMix;
  }
  return src + (combined - src) * p.outputMix;
}

kernel void ODTN6ColorKernel(constant int &width [[ buffer(0) ]],
                             constant int &height [[ buffer(1) ]],
                             constant int &rowBytes [[ buffer(2) ]],
                             constant ODTN6ColorParams &params [[ buffer(3) ]],
                             const device float *input [[ buffer(4) ]],
                             device float *output [[ buffer(5) ]],
                             uint2 id [[ thread_position_in_grid ]]) {
  if (id.x >= (uint)width || id.y >= (uint)height) return;
  int fpr = rowBytes / int(sizeof(float));
  int idx = int(id.y) * fpr + int(id.x) * 4;
  float3 src = float3(input[idx + 0], input[idx + 1], input[idx + 2]);
  float3 out = applyN6Color(src, params);
  output[idx + 0] = out.x;
  output[idx + 1] = out.y;
  output[idx + 2] = out.z;
  output[idx + 3] = input[idx + 3];
}
)METAL";

static std::mutex s_Mutex;
static std::unordered_map<id<MTLCommandQueue>, id<MTLComputePipelineState>>
    s_PipelineMap;

extern "C" void RunODTN6ColorMetalKernel(void *commandQueue, int width,
                                         int height, int rowBytes,
                                         const float *input, float *output,
                                         ODTN6ColorParams params) {
  @autoreleasepool {
    id<MTLCommandQueue> queue = static_cast<id<MTLCommandQueue>>(commandQueue);
    if (!queue) {
      return;
    }

    id<MTLDevice> device = queue.device;
    id<MTLComputePipelineState> pipelineState = nil;
    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      auto it = s_PipelineMap.find(queue);
      if (it != s_PipelineMap.end()) {
        pipelineState = it->second;
      } else {
        NSError *err = nil;
        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 150000
        options.mathMode = MTLMathModeFast;
#else
        options.fastMathEnabled = YES;
#endif

        id<MTLLibrary> library =
            [device newLibraryWithSource:@(kMetalSource)
                                  options:options
                                    error:&err];
        [options release];
        if (!library) {
          fprintf(stderr, "[ODTN6Color] Metal compile error: %s\n",
                  err.localizedDescription.UTF8String);
          return;
        }

        id<MTLFunction> fn = [library newFunctionWithName:@"ODTN6ColorKernel"];
        [library release];
        if (!fn) {
          fprintf(stderr,
                  "[ODTN6Color] Metal function ODTN6ColorKernel not found\n");
          return;
        }

        pipelineState = [device newComputePipelineStateWithFunction:fn
                                                              error:&err];
        [fn release];
        if (!pipelineState) {
          fprintf(stderr, "[ODTN6Color] Metal pipeline error: %s\n",
                  err.localizedDescription.UTF8String);
          return;
        }
        s_PipelineMap[queue] = pipelineState;
      }
    }

    id<MTLBuffer> srcBuf =
        reinterpret_cast<id<MTLBuffer>>(const_cast<float *>(input));
    id<MTLBuffer> dstBuf = reinterpret_cast<id<MTLBuffer>>(output);

    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [cmdBuf computeCommandEncoder];
    [encoder setComputePipelineState:pipelineState];
    [encoder setBytes:&width length:sizeof(int) atIndex:0];
    [encoder setBytes:&height length:sizeof(int) atIndex:1];
    [encoder setBytes:&rowBytes length:sizeof(int) atIndex:2];
    [encoder setBytes:&params length:sizeof(ODTN6ColorParams) atIndex:3];
    [encoder setBuffer:srcBuf offset:0 atIndex:4];
    [encoder setBuffer:dstBuf offset:0 atIndex:5];

    NSUInteger exeWidth = pipelineState.threadExecutionWidth;
    NSUInteger maxHeight =
        pipelineState.maxTotalThreadsPerThreadgroup / exeWidth;
    MTLSize threadsPerGroup = MTLSizeMake(exeWidth, maxHeight, 1);
    MTLSize threadgroups =
        MTLSizeMake(((NSUInteger)width + exeWidth - 1) / exeWidth,
                    ((NSUInteger)height + maxHeight - 1) / maxHeight, 1);

    [encoder dispatchThreadgroups:threadgroups
            threadsPerThreadgroup:threadsPerGroup];
    [encoder endEncoding];
    [cmdBuf commit];
  }
}
