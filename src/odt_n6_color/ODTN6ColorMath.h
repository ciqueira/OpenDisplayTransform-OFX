// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.
// Portions of the image math are derived from open-display-transform by Jed Smith.

#ifndef ODT_N6_COLOR_MATH_H
#define ODT_N6_COLOR_MATH_H

#include "ODTN6ColorParams.h"

#include <algorithm>
#include <cmath>

#ifdef __CUDACC__
#define ODT_N6_HD __host__ __device__
#else
#define ODT_N6_HD
#endif

namespace odtn6color {

struct float3 {
  float x;
  float y;
  float z;
};

ODT_N6_HD inline float n6Min(float a, float b) { return a < b ? a : b; }
ODT_N6_HD inline float n6Max(float a, float b) { return a > b ? a : b; }
ODT_N6_HD inline float n6Abs(float a) { return fabsf(a); }
ODT_N6_HD inline float n6Pow(float a, float b) { return powf(a, b); }
ODT_N6_HD inline float n6Fmod(float a, float b) { return fmodf(a, b); }
ODT_N6_HD inline float n6Exp2(float a) { return exp2f(a); }
ODT_N6_HD inline float n6Log2(float a) { return log2f(a); }
ODT_N6_HD inline float n6Log10(float a) { return log10f(a); }

ODT_N6_HD inline float3 make_float3(float x, float y, float z) { return {x, y, z}; }
ODT_N6_HD inline float3 make_float3(float v) { return {v, v, v}; }
ODT_N6_HD inline float3 operator+(float3 a, float3 b) {
  return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
ODT_N6_HD inline float3 operator+(float a, float3 b) { return make_float3(a) + b; }
ODT_N6_HD inline float3 operator+(float3 a, float b) { return a + make_float3(b); }
ODT_N6_HD inline float3 operator-(float3 a, float3 b) {
  return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
ODT_N6_HD inline float3 operator*(float3 a, float b) {
  return make_float3(a.x * b, a.y * b, a.z * b);
}
ODT_N6_HD inline float3 operator*(float b, float3 a) { return a * b; }
ODT_N6_HD inline float3 operator/(float3 a, float b) {
  return make_float3(a.x / b, a.y / b, a.z / b);
}

ODT_N6_HD inline float clampf(float v, float lo, float hi) {
  return n6Min(n6Max(v, lo), hi);
}

ODT_N6_HD inline float sdivf(float a, float b) {
  return n6Abs(b) < 0.0001f ? 0.0f : a / b;
}

ODT_N6_HD inline float3 sdivf3f(float3 a, float b) {
  return make_float3(sdivf(a.x, b), sdivf(a.y, b), sdivf(a.z, b));
}

ODT_N6_HD inline float spowf(float a, float b) {
  return a <= 0.0f ? a : n6Pow(a, b);
}

ODT_N6_HD inline float exp10f_compat(float x) { return n6Pow(10.0f, x); }

ODT_N6_HD inline float extract(float e0, float e1, float x) {
  return clampf((x - e0) / (e1 - e0), 0.0f, 1.0f);
}

ODT_N6_HD inline float extractWindow(float e0, float e1, float e2, float e3, float x) {
  return x < e1 ? extract(e0, e1, x) : extract(e3, e2, x);
}

ODT_N6_HD inline float3 zoneExtract(float3 in, float3 rgb, float zp, int zr) {
  const float n = n6Max(1e-12f, n6Max(rgb.x, n6Max(rgb.y, rgb.z)));
  const float fl = 0.01f;
  const float zpow = n6Pow(2.0f, -zp + 1.0f);
  const float toe = (n * n / (n + fl));
  float f = n6Pow((toe / (toe + 1.0f)) / n, zpow);
  if (zr == 1) {
    f = 1.0f - n6Pow((n / (n + 1.0f)) / n, zpow);
  }
  return in * (1.0f - f) + rgb * f;
}

ODT_N6_HD inline float extractHueAngle(float h, float o, float w, int sm) {
  float hc = extractWindow(2.0f - w, 2.0f, 2.0f, 2.0f + w,
                           n6Fmod(h + o, 6.0f));
  if (sm == 1) {
    hc = hc * hc * (3.0f - 2.0f * hc);
  }
  return hc;
}

ODT_N6_HD inline float calcHue(float3 rgb) {
  const float mx = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  const float mn = n6Min(rgb.x, n6Min(rgb.y, rgb.z));
  const float ch = mx - mn;
  if (ch == 0.0f) {
    return 0.0f;
  }
  if (mx == rgb.x) {
    return n6Fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
  }
  if (mx == rgb.y) {
    return (rgb.z - rgb.x) / ch + 2.0f;
  }
  return (rgb.x - rgb.y) / ch + 4.0f;
}

ODT_N6_HD inline float calcChroma(float3 rgb) {
  const float mx = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  const float mn = n6Min(rgb.x, n6Min(rgb.y, rgb.z));
  return sdivf(mx - mn, mx);
}

ODT_N6_HD inline float oetfDavinciIntermediate(float x, int inv) {
  if (inv == 1) {
    return x <= 0.02740668f ? x / 10.44426855f
                            : n6Exp2(x / 0.07329248f - 7.0f) - 0.0075f;
  }
  return x <= 0.00262409f
             ? x * 10.44426855f
             : (n6Log2(x + 0.0075f) + 7.0f) * 0.07329248f;
}

ODT_N6_HD inline float oetfAcescct(float x, int inv) {
  if (inv == 1) {
    return x <= 0.155251141552511f
               ? (x - 0.0729055341958355f) / 10.5402377416545f
               : n6Exp2(x * 17.52f - 9.72f);
  }
  return x <= 0.0078125f ? 10.5402377416545f * x + 0.0729055341958355f
                         : (n6Log2(x) + 9.72f) / 17.52f;
}

ODT_N6_HD inline float oetfArriLogC3(float x, int inv) {
  if (inv == 1) {
    return x < 5.367655f * 0.010591f + 0.092809f
               ? (x - 0.092809f) / 5.367655f
               : (exp10f_compat((x - 0.385537f) / 0.247190f) - 0.052272f) /
                     5.555556f;
  }
  return x < 0.010591f
             ? 5.367655f * x + 0.092809f
             : 0.247190f * n6Log10(5.555556f * x + 0.052272f) +
                   0.385537f;
}

ODT_N6_HD inline float oetfArriLogC4(float x, int inv) {
  if (inv == 1) {
    return x < -0.7774983977293537f
               ? x * 0.3033266726886969f - 0.7774983977293537f
               : (n6Exp2(14.0f * (x - 0.09286412512218964f) /
                                 0.9071358748778103f +
                             6.0f) -
                  64.0f) /
                     2231.8263090676883f;
  }
  return x < -0.7774983977293537f
             ? (x - -0.7774983977293537f) / 0.3033266726886969f
             : (n6Log2(2231.8263090676883f * x + 64.0f) - 6.0f) /
                       14.0f * 0.9071358748778103f +
                   0.09286412512218964f;
}

ODT_N6_HD inline float oetfRedLog3G10(float x, int inv) {
  const float a = 0.224282f;
  const float b = 155.975327f;
  const float c = 0.01f;
  const float g = 15.1927f;
  if (inv == 1) {
    return x < 0.0f ? (x / g) - c : (exp10f_compat(x / a) - 1.0f) / b - c;
  }
  return x < -c ? (x + c) * g : a * n6Log10((x + c) * b + 1.0f);
}

ODT_N6_HD inline float3 linearize(float3 rgb, int tf, int inv) {
  switch (tf) {
  case 0:
    return rgb;
  case 1:
    return make_float3(oetfDavinciIntermediate(rgb.x, !inv),
                       oetfDavinciIntermediate(rgb.y, !inv),
                       oetfDavinciIntermediate(rgb.z, !inv));
  case 2:
    return make_float3(oetfAcescct(rgb.x, !inv), oetfAcescct(rgb.y, !inv),
                       oetfAcescct(rgb.z, !inv));
  case 3:
    return make_float3(oetfArriLogC3(rgb.x, !inv),
                       oetfArriLogC3(rgb.y, !inv),
                       oetfArriLogC3(rgb.z, !inv));
  case 4:
    return make_float3(oetfArriLogC4(rgb.x, !inv),
                       oetfArriLogC4(rgb.y, !inv),
                       oetfArriLogC4(rgb.z, !inv));
  case 5:
    return make_float3(oetfRedLog3G10(rgb.x, !inv),
                       oetfRedLog3G10(rgb.y, !inv),
                       oetfRedLog3G10(rgb.z, !inv));
  default:
    return rgb;
  }
}

ODT_N6_HD inline float3 n6ChromaValue(float3 rgb, float my, float mr, float mm, float mb,
                            float mc, float mg, float hsRgb, float hsCmy,
                            float chs, float chl, int ze, float zp, int zr) {
  const float3 mp =
      make_float3(n6Pow(2.0f, mr), n6Pow(2.0f, mg), n6Pow(2.0f, mb));
  const float3 ms =
      make_float3(n6Pow(2.0f, mc), n6Pow(2.0f, mm), n6Pow(2.0f, my));

  const float3 in = rgb;
  const float hue = calcHue(rgb);
  const float ch = calcChroma(rgb);

  float chStr = spowf(n6Min(1.0f, ch), sdivf(1.0f, chs));
  chStr = chStr * spowf(1.0f - chStr, chl);

  const float3 hp = make_float3(extractHueAngle(hue, 2.0f, 1.0f, 0),
                                extractHueAngle(hue, 6.0f, 1.0f, 0),
                                extractHueAngle(hue, 4.0f, 1.0f, 0));
  const float3 hs = make_float3(extractHueAngle(hue, 5.0f, 1.0f, 0),
                                extractHueAngle(hue, 3.0f, 1.0f, 0),
                                extractHueAngle(hue, 1.0f, 1.0f, 0));
  const float3 pp = make_float3(n6Min(hsRgb, hsRgb / mp.x),
                                n6Min(hsRgb, hsRgb / mp.y),
                                n6Min(hsRgb, hsRgb / mp.z));
  const float3 ps = make_float3(n6Min(hsCmy, hsCmy / ms.x),
                                n6Min(hsCmy, hsCmy / ms.y),
                                n6Min(hsCmy, hsCmy / ms.z));
  const float3 hpW = make_float3(1.0f - spowf(1.0f - hp.x, pp.x),
                                 1.0f - spowf(1.0f - hp.y, pp.y),
                                 1.0f - spowf(1.0f - hp.z, pp.z));
  const float3 hsW = make_float3(1.0f - spowf(1.0f - hs.x, ps.x),
                                 1.0f - spowf(1.0f - hs.y, ps.y),
                                 1.0f - spowf(1.0f - hs.z, ps.z));

  const float mf = ((1.0f - hpW.x) + mp.x * hpW.x) *
                   ((1.0f - hpW.y) + mp.y * hpW.y) *
                   ((1.0f - hpW.z) + mp.z * hpW.z) *
                   ((1.0f - hsW.x) + ms.x * hsW.x) *
                   ((1.0f - hsW.y) + ms.y * hsW.y) *
                   ((1.0f - hsW.z) + ms.z * hsW.z);

  const float mfLim = n6Max(1.0f, mf);
  const float chf = sdivf(chStr, (chStr * (1.0f - mfLim) + mfLim));
  rgb = rgb * mf * chf + rgb * (1.0f - chf);

  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

ODT_N6_HD inline float3 n6Vibrance(float3 rgb, float mgl, float my, float mr, float mm,
                         float mb, float mc, float mg, float mcu, float cuh,
                         float cuw, float hl, int ze, float zp, int zr) {
  const float3 in = rgb;
  const float pBase = 3.0f;
  const float mglStr = n6Pow(pBase, mgl);
  const float3 pp = make_float3(n6Pow(pBase, mr) * mglStr,
                                n6Pow(pBase, mg) * mglStr,
                                n6Pow(pBase, mb) * mglStr);
  const float3 ps = make_float3(n6Pow(pBase, mc) * mglStr,
                                n6Pow(pBase, mm) * mglStr,
                                n6Pow(pBase, my) *
                                    n6Pow(pBase / 1.5f, mgl));
  const float pc = n6Pow(pBase, mcu);

  const float n = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  float3 r = n == 0.0f ? make_float3(0.0f, 0.0f, 0.0f) : rgb / n;
  r = make_float3(n6Max(r.x, -1.0f), n6Max(r.y, -1.0f),
                  n6Max(r.z, -1.0f));

  const float h = calcHue(r);
  const float c = 1.0f - n6Min(r.x, n6Min(r.y, r.z));
  float3 rHl = r;

  float3 hp = make_float3(extractHueAngle(h, 2.0f, 1.0f, 0),
                          extractHueAngle(h, 6.0f, 1.0f, 0),
                          extractHueAngle(h, 4.0f, 1.0f, 0)) *
              c;
  float3 hs = make_float3(extractHueAngle(h, 5.0f, 1.0f, 0),
                          extractHueAngle(h, 3.0f, 1.0f, 0),
                          extractHueAngle(h, 1.0f, 1.0f, 0)) *
              c;
  const float hc = extractHueAngle(h, cuh / 60.0f, cuw, 0) * c;

  r.x = r.x < 0.0f ? r.x
                   : n6Pow(r.x, pp.x) * hp.x + n6Pow(r.x, pp.y) * hp.y +
                         n6Pow(r.x, pp.z) * hp.z +
                         r.x * (1.0f - (hp.x + hp.y + hp.z));
  r.y = r.y < 0.0f ? r.y
                   : n6Pow(r.y, pp.x) * hp.x + n6Pow(r.y, pp.y) * hp.y +
                         n6Pow(r.y, pp.z) * hp.z +
                         r.y * (1.0f - (hp.x + hp.y + hp.z));
  r.z = r.z < 0.0f ? r.z
                   : n6Pow(r.z, pp.x) * hp.x + n6Pow(r.z, pp.y) * hp.y +
                         n6Pow(r.z, pp.z) * hp.z +
                         r.z * (1.0f - (hp.x + hp.y + hp.z));

  r.x = r.x < 0.0f ? r.x
                   : n6Pow(r.x, ps.x) * hs.x + n6Pow(r.x, ps.y) * hs.y +
                         n6Pow(r.x, ps.z) * hs.z +
                         r.x * (1.0f - (hs.x + hs.y + hs.z));
  r.y = r.y < 0.0f ? r.y
                   : n6Pow(r.y, ps.x) * hs.x + n6Pow(r.y, ps.y) * hs.y +
                         n6Pow(r.y, ps.z) * hs.z +
                         r.y * (1.0f - (hs.x + hs.y + hs.z));
  r.z = r.z < 0.0f ? r.z
                   : n6Pow(r.z, ps.x) * hs.x + n6Pow(r.z, ps.y) * hs.y +
                         n6Pow(r.z, ps.z) * hs.z +
                         r.z * (1.0f - (hs.x + hs.y + hs.z));

  r.x = r.x < 0.0f ? r.x : n6Pow(r.x, pc) * hc + r.x * (1.0f - hc);
  r.y = r.y < 0.0f ? r.y : n6Pow(r.y, pc) * hc + r.y * (1.0f - hc);
  r.z = r.z < 0.0f ? r.z : n6Pow(r.z, pc) * hc + r.z * (1.0f - hc);

  float m = 1.1f;
  float vf = c == 0.0f || c > 1.0f
                 ? 1.0f
                 : ((1.0f - n6Pow(1.0f - c, pp.x)) * hp.x +
                    (1.0f - n6Pow(1.0f - c, pp.y)) * hp.y +
                    (1.0f - n6Pow(1.0f - c, pp.z)) * hp.z +
                    c * (1.0f - (hp.x + hp.y + hp.z))) /
                       c;
  rHl = m * (1.0f - vf) + rHl * vf;

  m = 1.0f;
  vf = c == 0.0f || c > 1.0f
           ? 1.0f
           : ((1.0f - n6Pow(1.0f - c, ps.x)) * hs.x +
              (1.0f - n6Pow(1.0f - c, ps.y)) * hs.y +
              (1.0f - n6Pow(1.0f - c, ps.z)) * hs.z +
              c * (1.0f - (hs.x + hs.y + hs.z))) /
                 c;
  rHl = m * (1.0f - vf) + rHl * vf;

  vf = c == 0.0f || c > 1.0f
           ? 1.0f
           : ((1.0f - n6Pow(1.0f - c, pc)) * hc + c * (1.0f - hc)) / c;
  rHl = m * (1.0f - vf) + rHl * vf;

  r = r * (1.0f - hl) + rHl * hl;
  rgb = r * n;
  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

ODT_N6_HD inline float3 n6HueShift(float3 rgb, float sy, float sr, float sm, float sb,
                         float sc, float sg, float scu, float cuh, float cuw,
                         float str, float chl, int ze, float zp, int zr) {
  const float3 in = rgb;
  const float n = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  float3 r = n == 0.0f ? make_float3(0.0f, 0.0f, 0.0f) : rgb / n;

  float ch = n6Min(r.x, n6Min(r.y, r.z)) * (1.0f - str) + str;
  ch = ch == 0.0f ? 0.0f
                  : n6Min(1.0f, 1.0f - n6Min(r.x / ch,
                                                    n6Min(r.y / ch, r.z / ch)));
  const float f0 =
      chl < 0.5f ? n6Max(0.0f, 0.5f - chl) * 2.0f
                 : 1.0f / n6Max(1e-3f, (1.0f - chl) * 2.0f);
  ch = chl < 0.5f ? ch * f0 + ch * (1.0f - ch) * (1.0f - f0)
                  : ch * n6Pow(n6Max(0.0f, 1.0f - ch), f0);

  const float hue = calcHue(r);
  const float3 hp = make_float3(extractHueAngle(hue, 2.0f, 1.0f, 0),
                                extractHueAngle(hue, 6.0f, 1.0f, 0),
                                extractHueAngle(hue, 4.0f, 1.0f, 0)) *
                    ch;
  const float3 hs = make_float3(extractHueAngle(hue, 5.0f, 1.0f, 0),
                                extractHueAngle(hue, 3.0f, 1.0f, 0),
                                extractHueAngle(hue, 1.0f, 1.0f, 0)) *
                    ch;

  r.x = r.x + sb * hp.z - sg * hp.y;
  r.y = r.y + sr * hp.x - sb * hp.z;
  r.z = r.z + sg * hp.y - sr * hp.x;

  r.x = r.x + sy * hs.z - sm * hs.y;
  r.y = r.y + sc * hs.x - sy * hs.z;
  r.z = r.z + sm * hs.y - sc * hs.x;

  const float hc = extractHueAngle(hue, cuh / 60.0f, cuw, 0) * ch;
  const float h = cuh / 60.0f;
  const float s = scu * cuw;
  const float sc0 =
      h < 3.0f ? s - s * n6Min(1.0f, n6Abs(h - 2.0f))
               : s * n6Min(1.0f, n6Abs(h - 5.0f)) - s;
  const float sc1 =
      h < 1.0f ? s - s * n6Min(1.0f, n6Abs(h))
               : h < 5.0f
                     ? s * n6Min(1.0f, n6Abs(h - 3.0f)) - s
                     : s - s * n6Min(1.0f, n6Abs(h - 6.0f));
  const float sc2 =
      h < 2.0f ? s * n6Min(1.0f, n6Abs(h - 1.0f)) - s
               : s - s * n6Min(1.0f, n6Abs(h - 4.0f));

  r.x = r.x + hc * (sc2 - sc1);
  r.y = r.y + hc * (sc0 - sc2);
  r.z = r.z + hc * (sc1 - sc0);

  rgb = r * n;
  return ze == 1 ? zoneExtract(in, rgb, zp, zr) : rgb;
}

ODT_N6_HD inline float powerWindow(float x, float c, float p0, float p1, float x0,
                         float x1) {
  return x > x1
             ? 0.0f
             : x > c
                   ? n6Pow(1.0f - n6Pow((x - c) / (x1 - c), p0), p1)
                   : x > x0
                         ? n6Pow(1.0f - n6Pow((c - x) / (c - x0), p0),
                                    p1)
                         : 0.0f;
}

ODT_N6_HD inline float pivotedPowerCubic(float x, float m, float p) {
  m = 1.0f - n6Abs(m);
  const float a0 = p * (m - 1.0f);
  const float b0 = (1.0f - m) * (p + 1.0f);
  return x <= 0.0f ? m * x
                   : x > 1.0f ? x
                               : x * (n6Pow(x, p) * (a0 * x + b0) + m);
}

ODT_N6_HD inline float3 n6Purity(float3 rgb, const ODTN6ColorParams &p) {
  const float mx = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  const float mn = n6Min(rgb.x, n6Min(rgb.y, rgb.z));
  float ch = mx - mn;
  float h = 0.0f;
  if (ch != 0.0f) {
    if (mx == rgb.x) {
      h = n6Fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
    } else if (mx == rgb.y) {
      h = (rgb.z - rgb.x) / ch + 2.0f;
    } else {
      h = (rgb.x - rgb.y) / ch + 4.0f;
    }
  }
  ch = sdivf(ch, mx);

  const float hr = powerWindow(n6Fmod(h + 1.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);
  const float hg = powerWindow(n6Fmod(h + 5.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);
  const float hb = powerWindow(n6Fmod(h + 3.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);
  const float hc = powerWindow(n6Fmod(h + 4.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);
  const float hm = powerWindow(n6Fmod(h + 2.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);
  const float hy = powerWindow(n6Fmod(h + 0.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f);

  const float cr = sdivf(p.purityRed < 0.0f
                             ? pivotedPowerCubic(ch, p.purityRed,
                                                 p.purityRedStrength)
                             : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityRed,
                                                        p.purityRedStrength),
                         ch);
  const float cg = sdivf(p.purityGreen < 0.0f
                             ? pivotedPowerCubic(ch, p.purityGreen,
                                                 p.purityGreenStrength)
                             : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityGreen,
                                                        p.purityGreenStrength),
                         ch);
  const float cb = sdivf(p.purityBlue < 0.0f
                             ? pivotedPowerCubic(ch, p.purityBlue,
                                                 p.purityBlueStrength)
                             : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityBlue,
                                                        p.purityBlueStrength),
                         ch);
  const float cc = sdivf(p.purityCyan < 0.0f
                             ? pivotedPowerCubic(ch, p.purityCyan,
                                                 p.purityCyanStrength)
                             : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityCyan,
                                                        p.purityCyanStrength),
                         ch);
  const float cm =
      sdivf(p.purityMagenta < 0.0f
                ? pivotedPowerCubic(ch, p.purityMagenta,
                                    p.purityMagentaStrength)
                : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityMagenta,
                                           p.purityMagentaStrength),
            ch);
  const float cy =
      sdivf(p.purityYellow < 0.0f
                ? pivotedPowerCubic(ch, p.purityYellow, p.purityYellowStrength)
                : 1.0f - pivotedPowerCubic(1.0f - ch, p.purityYellow,
                                           p.purityYellowStrength),
            ch);

  const float chf = (hr * cr + hg * cg + hb * cb +
                     (1.0f - (hr + hg + hb))) *
                    (hc * cc + hm * cm + hy * cy +
                     (1.0f - (hc + hm + hy)));
  const float wsum = rgb.x * 0.3f + rgb.y * 0.6f + rgb.z * 0.1f;
  return make_float3(wsum, wsum, wsum) * (1.0f - chf) + rgb * chf;
}

ODT_N6_HD inline float3 maxf3(float3 a, float b) {
  return make_float3(n6Max(a.x, b), n6Max(a.y, b), n6Max(a.z, b));
}

ODT_N6_HD inline float3 n6CrossTalk(float3 rgb, const ODTN6ColorParams &p) {
  float mx = n6Max(rgb.x, n6Max(rgb.y, rgb.z));
  const float mn = n6Min(rgb.x, n6Min(rgb.y, rgb.z));
  float ch = mx - mn;
  float h = 0.0f;
  if (ch != 0.0f) {
    if (mx == rgb.x) {
      h = n6Fmod((rgb.y - rgb.z) / ch + 6.0f, 6.0f);
    } else if (mx == rgb.y) {
      h = (rgb.z - rgb.x) / ch + 2.0f;
    } else {
      h = (rgb.x - rgb.y) / ch + 4.0f;
    }
  }
  ch = clampf(sdivf(ch, mx), 0.0f, 4.0f);
  const float hch = mn * 0.5f + mx * 0.5f;
  rgb = maxf3(sdivf3f(rgb, hch), -2.0f);

  const float hr = powerWindow(n6Fmod(h + 1.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f) *
                   spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkRedPower));
  const float hg = powerWindow(n6Fmod(h + 5.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f) *
                   spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkGreenPower));
  const float hb = powerWindow(n6Fmod(h + 3.0f, 6.0f), 1.0f, 1.5f, 1.5f,
                               0.0f, 2.0f) *
                   spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkBluePower));
  const float hc =
      powerWindow(n6Fmod(h + 4.0f, 6.0f), 1.0f + p.crossTalkCyanCenter,
                  1.5f, 1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkCyanPower));
  const float hm =
      powerWindow(n6Fmod(h + 2.0f, 6.0f), 1.0f + p.crossTalkMagentaCenter,
                  1.5f, 1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkMagentaPower));
  const float hy =
      powerWindow(n6Fmod(h + 0.0f, 6.0f), 1.0f + p.crossTalkYellowCenter,
                  1.5f, 1.5f, 0.0f, 2.0f) *
      spowf(ch, 1.0f / n6Max(1e-5f, p.crossTalkYellowPower));

  rgb.x = (p.crossTalkMagentaScale + 1.0f) *
              (rgb.x - n6Max(0.0f, p.crossTalkMagentaShift0) -
               n6Min(0.0f, p.crossTalkMagentaShift1)) *
              hm +
          (p.crossTalkYellowScale + 1.0f) *
              (rgb.x + n6Min(0.0f, p.crossTalkYellowShift0) +
               n6Max(0.0f, p.crossTalkYellowShift1)) *
              hy +
          rgb.x * (1.0f - (hm + hy));
  rgb.y = (p.crossTalkCyanScale + 1.0f) *
              (rgb.y - n6Max(0.0f, p.crossTalkCyanShift0) -
               n6Min(0.0f, p.crossTalkCyanShift1)) *
              hc +
          (p.crossTalkYellowScale + 1.0f) *
              (rgb.y - n6Max(0.0f, p.crossTalkYellowShift0) -
               n6Min(0.0f, p.crossTalkYellowShift1)) *
              hy +
          rgb.y * (1.0f - (hc + hy));
  rgb.z = (p.crossTalkCyanScale + 1.0f) *
              (rgb.z + n6Min(0.0f, p.crossTalkCyanShift0) +
               n6Max(0.0f, p.crossTalkCyanShift1)) *
              hc +
          (p.crossTalkMagentaScale + 1.0f) *
              (rgb.z + n6Min(0.0f, p.crossTalkMagentaShift0) +
               n6Max(0.0f, p.crossTalkMagentaShift1)) *
              hm +
          rgb.z * (1.0f - (hc + hm));

  rgb.x = (p.crossTalkRedScale + 1.0f) * rgb.x * hr +
          rgb.x * (1.0f + n6Min(0.0f, p.crossTalkGreenShift0)) *
              (1.0f + n6Max(0.0f, p.crossTalkGreenShift1)) * hg +
          rgb.x * (1.0f - n6Max(0.0f, p.crossTalkBlueShift0)) *
              (1.0f - n6Min(0.0f, p.crossTalkBlueShift1)) * hb +
          rgb.x * (1.0f - (hr + hg + hb));
  rgb.y = (p.crossTalkGreenScale + 1.0f) * rgb.y * hg +
          rgb.y * (1.0f - n6Max(0.0f, p.crossTalkRedShift0)) *
              (1.0f - n6Min(0.0f, p.crossTalkRedShift1)) * hr +
          rgb.y * (1.0f + n6Min(0.0f, p.crossTalkBlueShift0)) *
              (1.0f + n6Max(0.0f, p.crossTalkBlueShift1)) * hb +
          rgb.y * (1.0f - (hr + hg + hb));
  rgb.z = (p.crossTalkBlueScale + 1.0f) * rgb.z * hb +
          rgb.z * (1.0f + n6Min(0.0f, p.crossTalkRedShift0)) *
              (1.0f + n6Max(0.0f, p.crossTalkRedShift1)) * hr +
          rgb.z * (1.0f - n6Max(0.0f, p.crossTalkGreenShift0)) *
              (1.0f - n6Min(0.0f, p.crossTalkGreenShift1)) * hg +
          rgb.z * (1.0f - (hr + hg + hb));

  return rgb * hch;
}

ODT_N6_HD inline bool nearZero(float v) { return n6Abs(v) <= 1.0e-6f; }
ODT_N6_HD inline bool nearValue(float v, float target) {
  return n6Abs(v - target) <= 1.0e-6f;
}

ODT_N6_HD inline bool purityNeutral(const ODTN6ColorParams &p) {
  return nearZero(p.purityRed) && nearZero(p.purityGreen) &&
         nearZero(p.purityBlue) && nearZero(p.purityCyan) &&
         nearZero(p.purityMagenta) && nearZero(p.purityYellow);
}

ODT_N6_HD inline bool chromaValueNeutral(const ODTN6ColorParams &p) {
  return nearZero(p.chromaValueYellow) && nearZero(p.chromaValueRed) &&
         nearZero(p.chromaValueMagenta) && nearZero(p.chromaValueBlue) &&
         nearZero(p.chromaValueCyan) && nearZero(p.chromaValueGreen);
}

ODT_N6_HD inline bool vibranceNeutral(const ODTN6ColorParams &p) {
  return nearZero(p.vibranceGlobal) && nearZero(p.vibranceYellow) &&
         nearZero(p.vibranceRed) && nearZero(p.vibranceMagenta) &&
         nearZero(p.vibranceBlue) && nearZero(p.vibranceCyan) &&
         nearZero(p.vibranceGreen) && nearZero(p.vibranceCustom);
}

ODT_N6_HD inline bool hueShiftNeutral(const ODTN6ColorParams &p) {
  return nearZero(p.hueShiftYellow) && nearZero(p.hueShiftRed) &&
         nearZero(p.hueShiftMagenta) && nearZero(p.hueShiftBlue) &&
         nearZero(p.hueShiftCyan) && nearZero(p.hueShiftGreen) &&
         nearZero(p.hueShiftCustom);
}

ODT_N6_HD inline bool crossTalkNeutral(const ODTN6ColorParams &p) {
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

ODT_N6_HD inline bool hasActiveN6ColorEffect(const ODTN6ColorParams &p) {
  if (nearZero(p.outputMix)) {
    return false;
  }
  return (p.enablePurity && !nearZero(p.purityMix) && !purityNeutral(p)) ||
         (p.enableChromaValue && !nearZero(p.chromaValueMix) &&
          !chromaValueNeutral(p)) ||
         (p.enableVibrance && !nearZero(p.vibranceMix) &&
          !vibranceNeutral(p)) ||
         (p.enableHueShift && !nearZero(p.hueShiftMix) &&
          !hueShiftNeutral(p)) ||
         (p.enableCrossTalk && !nearZero(p.crossTalkMix) &&
          !crossTalkNeutral(p));
}

ODT_N6_HD inline float3 applyN6Color(float3 src, const ODTN6ColorParams &p) {
  if (!hasActiveN6ColorEffect(p)) {
    return src;
  }

  float3 combined = src;

  if (p.enablePurity && !nearZero(p.purityMix) && !purityNeutral(p)) {
    const float3 branch = n6Purity(src, p);
    combined = combined + (branch - src) * p.purityMix;
  }
  if (p.enableChromaValue && !nearZero(p.chromaValueMix) &&
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
    combined = combined + (branch - src) * p.chromaValueMix;
  }
  if (p.enableVibrance && !nearZero(p.vibranceMix) && !vibranceNeutral(p)) {
    float3 branch = linearize(src, p.transferFunction, 0);
    branch = n6Vibrance(branch, p.vibranceGlobal, p.vibranceYellow,
                        p.vibranceRed, p.vibranceMagenta, p.vibranceBlue,
                        p.vibranceCyan, p.vibranceGreen, p.vibranceCustom,
                        p.vibranceCustomHue, p.vibranceCustomWidth,
                        p.vibranceHueLinear, p.vibranceZoneEnable,
                        p.vibranceZoneRange, p.vibranceZoneTarget);
    branch = linearize(branch, p.transferFunction, 1);
    combined = combined + (branch - src) * p.vibranceMix;
  }
  if (p.enableHueShift && !nearZero(p.hueShiftMix) && !hueShiftNeutral(p)) {
    float3 branch = linearize(src, p.transferFunction, 0);
    branch = n6HueShift(branch, p.hueShiftYellow, p.hueShiftRed,
                        p.hueShiftMagenta, p.hueShiftBlue, p.hueShiftCyan,
                        p.hueShiftGreen, p.hueShiftCustom,
                        p.hueShiftCustomHue, p.hueShiftCustomWidth,
                        p.hueShiftStrength, p.hueShiftChromaLimit,
                        p.hueShiftZoneEnable, p.hueShiftZoneRange,
                        p.hueShiftZoneTarget);
    branch = linearize(branch, p.transferFunction, 1);
    combined = combined + (branch - src) * p.hueShiftMix;
  }
  if (p.enableCrossTalk && !nearZero(p.crossTalkMix) && !crossTalkNeutral(p)) {
    const float3 branch = n6CrossTalk(src, p);
    combined = combined + (branch - src) * p.crossTalkMix;
  }

  return src + (combined - src) * p.outputMix;
}

} // namespace odtn6color

#endif // ODT_N6_COLOR_MATH_H
