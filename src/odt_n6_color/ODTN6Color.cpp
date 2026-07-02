// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.
// Portions of the image math are derived from open-display-transform by Jed Smith.

#include "ODTN6Color.h"

#include "ODTN6ColorMath.h"
#include "ODTN6ColorParams.h"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

#include "ofxsImageEffect.h"
#include "ofxsMultiThread.h"
#include "ofxsProcessing.h"

#ifndef PLUGIN_VERSION
#define kPluginVersion "v0.1.0"
#else
#define kPluginVersion PLUGIN_VERSION
#endif

#define kPluginName "ODT N6 Color"
#define kPluginNameLabel "ODT N6 Color " kPluginVersion
#define kPluginGrouping "Open Display Transform"
#define kPluginDescription                                                   \
  "Parallel OFX port of the Open Display Transform N6 color DCTL tools: "    \
  "Purity, Chroma Value, Vibrance, Hue Shift, and CrossTalk."
#define kPluginIdentifier "com.OpenDisplayTransform.OdtN6Color"
#define kPluginVersionMajor 1
#define kPluginVersionMinor 0

#define kSupportsTiles false
#define kSupportsMultiResolution false
#define kSupportsMultipleClipPARs false

#define kParamTransferFunction "n6_transfer_function"
#define kParamOutputMix "n6_output_mix"

#define kParamEnablePurity "n6_enable_purity"
#define kParamEnableChromaValue "n6_enable_chroma_value"
#define kParamEnableVibrance "n6_enable_vibrance"
#define kParamEnableHueShift "n6_enable_hue_shift"
#define kParamEnableCrossTalk "n6_enable_crosstalk"

#define kParamPurityMix "n6_purity_mix"
#define kParamChromaValueMix "n6_chroma_value_mix"
#define kParamVibranceMix "n6_vibrance_mix"
#define kParamHueShiftMix "n6_hue_shift_mix"
#define kParamCrossTalkMix "n6_crosstalk_mix"

#define kParamPurityRed "n6_purity_red"
#define kParamPurityRedStrength "n6_purity_red_strength"
#define kParamPurityGreen "n6_purity_green"
#define kParamPurityGreenStrength "n6_purity_green_strength"
#define kParamPurityBlue "n6_purity_blue"
#define kParamPurityBlueStrength "n6_purity_blue_strength"
#define kParamPurityCyan "n6_purity_cyan"
#define kParamPurityCyanStrength "n6_purity_cyan_strength"
#define kParamPurityMagenta "n6_purity_magenta"
#define kParamPurityMagentaStrength "n6_purity_magenta_strength"
#define kParamPurityYellow "n6_purity_yellow"
#define kParamPurityYellowStrength "n6_purity_yellow_strength"

#define kParamChromaValueYellow "n6_chroma_value_yellow"
#define kParamChromaValueRed "n6_chroma_value_red"
#define kParamChromaValueMagenta "n6_chroma_value_magenta"
#define kParamChromaValueBlue "n6_chroma_value_blue"
#define kParamChromaValueCyan "n6_chroma_value_cyan"
#define kParamChromaValueGreen "n6_chroma_value_green"
#define kParamChromaValueHueStrengthRgb "n6_chroma_value_hue_strength_rgb"
#define kParamChromaValueHueStrengthCmy "n6_chroma_value_hue_strength_cmy"
#define kParamChromaValueChromaStrength "n6_chroma_value_chroma_strength"
#define kParamChromaValueChromaLimit "n6_chroma_value_chroma_limit"
#define kParamChromaValueZoneEnable "n6_chroma_value_zone_enable"
#define kParamChromaValueZoneRange "n6_chroma_value_zone_range"
#define kParamChromaValueZoneTarget "n6_chroma_value_zone_target"

#define kParamVibranceGlobal "n6_vibrance_global"
#define kParamVibranceYellow "n6_vibrance_yellow"
#define kParamVibranceRed "n6_vibrance_red"
#define kParamVibranceMagenta "n6_vibrance_magenta"
#define kParamVibranceBlue "n6_vibrance_blue"
#define kParamVibranceCyan "n6_vibrance_cyan"
#define kParamVibranceGreen "n6_vibrance_green"
#define kParamVibranceCustom "n6_vibrance_custom"
#define kParamVibranceCustomHue "n6_vibrance_custom_hue"
#define kParamVibranceCustomWidth "n6_vibrance_custom_width"
#define kParamVibranceHueLinear "n6_vibrance_hue_linear"
#define kParamVibranceZoneEnable "n6_vibrance_zone_enable"
#define kParamVibranceZoneRange "n6_vibrance_zone_range"
#define kParamVibranceZoneTarget "n6_vibrance_zone_target"

#define kParamHueShiftYellow "n6_hue_shift_yellow"
#define kParamHueShiftRed "n6_hue_shift_red"
#define kParamHueShiftMagenta "n6_hue_shift_magenta"
#define kParamHueShiftBlue "n6_hue_shift_blue"
#define kParamHueShiftCyan "n6_hue_shift_cyan"
#define kParamHueShiftGreen "n6_hue_shift_green"
#define kParamHueShiftCustom "n6_hue_shift_custom"
#define kParamHueShiftCustomHue "n6_hue_shift_custom_hue"
#define kParamHueShiftCustomWidth "n6_hue_shift_custom_width"
#define kParamHueShiftStrength "n6_hue_shift_strength"
#define kParamHueShiftChromaLimit "n6_hue_shift_chroma_limit"
#define kParamHueShiftZoneEnable "n6_hue_shift_zone_enable"
#define kParamHueShiftZoneRange "n6_hue_shift_zone_range"
#define kParamHueShiftZoneTarget "n6_hue_shift_zone_target"

#define kParamCrossTalkYellowPower "n6_crosstalk_yellow_power"
#define kParamCrossTalkYellowShift0 "n6_crosstalk_yellow_shift0"
#define kParamCrossTalkYellowShift1 "n6_crosstalk_yellow_shift1"
#define kParamCrossTalkYellowScale "n6_crosstalk_yellow_scale"
#define kParamCrossTalkRedPower "n6_crosstalk_red_power"
#define kParamCrossTalkRedShift0 "n6_crosstalk_red_shift0"
#define kParamCrossTalkRedShift1 "n6_crosstalk_red_shift1"
#define kParamCrossTalkRedScale "n6_crosstalk_red_scale"
#define kParamCrossTalkMagentaPower "n6_crosstalk_magenta_power"
#define kParamCrossTalkMagentaShift0 "n6_crosstalk_magenta_shift0"
#define kParamCrossTalkMagentaShift1 "n6_crosstalk_magenta_shift1"
#define kParamCrossTalkMagentaScale "n6_crosstalk_magenta_scale"
#define kParamCrossTalkBluePower "n6_crosstalk_blue_power"
#define kParamCrossTalkBlueShift0 "n6_crosstalk_blue_shift0"
#define kParamCrossTalkBlueShift1 "n6_crosstalk_blue_shift1"
#define kParamCrossTalkBlueScale "n6_crosstalk_blue_scale"
#define kParamCrossTalkCyanPower "n6_crosstalk_cyan_power"
#define kParamCrossTalkCyanShift0 "n6_crosstalk_cyan_shift0"
#define kParamCrossTalkCyanShift1 "n6_crosstalk_cyan_shift1"
#define kParamCrossTalkCyanScale "n6_crosstalk_cyan_scale"
#define kParamCrossTalkGreenPower "n6_crosstalk_green_power"
#define kParamCrossTalkGreenShift0 "n6_crosstalk_green_shift0"
#define kParamCrossTalkGreenShift1 "n6_crosstalk_green_shift1"
#define kParamCrossTalkGreenScale "n6_crosstalk_green_scale"
#define kParamCrossTalkCyanCenter "n6_crosstalk_cyan_center"
#define kParamCrossTalkMagentaCenter "n6_crosstalk_magenta_center"
#define kParamCrossTalkYellowCenter "n6_crosstalk_yellow_center"

#define kParamAboutHelp "aboutHelp"
#define kParamAppMCNexus "appMCNexus"
#define kAboutHelpUrl "https://github.com/ciqueira/OpenDisplayTransform-OFX"

static void openExternalUrl(const char *url) {
#ifdef _WIN32
  ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
  std::string command = "open \"";
  command += url;
  command += "\" >/dev/null 2>&1";
  std::system(command.c_str());
#else
  std::string command = "xdg-open \"";
  command += url;
  command += "\" >/dev/null 2>&1 &";
  std::system(command.c_str());
#endif
}

static void openMCNexusApp() {
#ifdef __APPLE__
  std::system("open -a MCNexus >/dev/null 2>&1 || open "
              "\"/Applications/MCNexus.app\" >/dev/null 2>&1");
#elif defined(_WIN32)
  ShellExecuteA(NULL, "open", "mc-nexus:", NULL, NULL, SW_SHOWNORMAL);
#endif
}

#ifdef __APPLE__
extern "C" void RunODTN6ColorMetalKernel(void *commandQueue, int width,
                                         int height, int rowBytes,
                                         const float *input, float *output,
                                         ODTN6ColorParams params);
#else
extern "C" void RunODTN6ColorCudaKernel(void *stream, int width, int height,
                                        int rowBytes, const float *input,
                                        float *output,
                                        ODTN6ColorParams params);
#endif

class ODTN6ColorProcessor : public OFX::ImageProcessor {
public:
  explicit ODTN6ColorProcessor(OFX::ImageEffect &instance)
      : OFX::ImageProcessor(instance), _srcImg(nullptr) {
    std::memset(&_params, 0, sizeof(_params));
  }

  void setSrcImg(OFX::Image *img) { _srcImg = img; }
  void setParams(const ODTN6ColorParams &params) { _params = params; }

  virtual void processImagesCuda() override {
#ifndef __APPLE__
    const OfxRectI &bounds = _srcImg->getBounds();
    const int width = bounds.x2 - bounds.x1;
    const int height = bounds.y2 - bounds.y1;
    const int rowBytes = _srcImg->getRowBytes();
    const float *input = static_cast<const float *>(_srcImg->getPixelData());
    float *output = static_cast<float *>(_dstImg->getPixelData());

    RunODTN6ColorCudaKernel(_pCudaStream, width, height, rowBytes, input,
                            output, _params);
#endif
  }

  virtual void processImagesMetal() override {
#ifdef __APPLE__
    const OfxRectI &bounds = _srcImg->getBounds();
    const int width = bounds.x2 - bounds.x1;
    const int height = bounds.y2 - bounds.y1;
    const int rowBytes = _srcImg->getRowBytes();
    const float *input = static_cast<const float *>(_srcImg->getPixelData());
    float *output = static_cast<float *>(_dstImg->getPixelData());

    RunODTN6ColorMetalKernel(_pMetalCmdQ, width, height, rowBytes, input,
                             output, _params);
#endif
  }

  virtual void multiThreadProcessImages(OfxRectI procWindow) override {
    using namespace odtn6color;

    const OfxRectI &bounds = _srcImg->getBounds();

    for (int y = procWindow.y1; y < procWindow.y2; ++y) {
      if (_effect.abort()) {
        break;
      }

      const float *srcRow =
          static_cast<const float *>(_srcImg->getPixelAddress(procWindow.x1, y));
      float *dstRow =
          static_cast<float *>(_dstImg->getPixelAddress(procWindow.x1, y));
      if (!srcRow || !dstRow) {
        continue;
      }

      const int rowWidth = procWindow.x2 - procWindow.x1;
      for (int x = 0; x < rowWidth; ++x) {
        const int pixelOffset = x * 4;
        const float3 src = make_float3(srcRow[pixelOffset + 0],
                                       srcRow[pixelOffset + 1],
                                       srcRow[pixelOffset + 2]);
        const float3 out = applyN6Color(src, _params);
        dstRow[pixelOffset + 0] = out.x;
        dstRow[pixelOffset + 1] = out.y;
        dstRow[pixelOffset + 2] = out.z;
        dstRow[pixelOffset + 3] = srcRow[pixelOffset + 3];
      }
    }
    (void)bounds;
  }

private:
  OFX::Image *_srcImg;
  ODTN6ColorParams _params;
};

class ODTN6ColorPlugin : public OFX::ImageEffect {
public:
  explicit ODTN6ColorPlugin(OfxImageEffectHandle handle);

  virtual void render(const OFX::RenderArguments &args) override;
  virtual bool isIdentity(const OFX::IsIdentityArguments &args,
                          OFX::Clip *&identityClip,
                          double &identityTime) override;
  virtual void changedParam(const OFX::InstanceChangedArgs &args,
                            const std::string &paramName) override;

private:
  ODTN6ColorParams getActiveParams(double time);
  void setupAndProcess(ODTN6ColorProcessor &processor,
                       const OFX::RenderArguments &args);

  OFX::Clip *m_SrcClip;
  OFX::Clip *m_DstClip;

  OFX::ChoiceParam *m_TransferFunction;
  OFX::DoubleParam *m_OutputMix;

  OFX::BooleanParam *m_EnablePurity;
  OFX::BooleanParam *m_EnableChromaValue;
  OFX::BooleanParam *m_EnableVibrance;
  OFX::BooleanParam *m_EnableHueShift;
  OFX::BooleanParam *m_EnableCrossTalk;

  OFX::DoubleParam *m_PurityMix;
  OFX::DoubleParam *m_ChromaValueMix;
  OFX::DoubleParam *m_VibranceMix;
  OFX::DoubleParam *m_HueShiftMix;
  OFX::DoubleParam *m_CrossTalkMix;

  OFX::DoubleParam *m_PurityRed;
  OFX::DoubleParam *m_PurityRedStrength;
  OFX::DoubleParam *m_PurityGreen;
  OFX::DoubleParam *m_PurityGreenStrength;
  OFX::DoubleParam *m_PurityBlue;
  OFX::DoubleParam *m_PurityBlueStrength;
  OFX::DoubleParam *m_PurityCyan;
  OFX::DoubleParam *m_PurityCyanStrength;
  OFX::DoubleParam *m_PurityMagenta;
  OFX::DoubleParam *m_PurityMagentaStrength;
  OFX::DoubleParam *m_PurityYellow;
  OFX::DoubleParam *m_PurityYellowStrength;

  OFX::DoubleParam *m_ChromaValueYellow;
  OFX::DoubleParam *m_ChromaValueRed;
  OFX::DoubleParam *m_ChromaValueMagenta;
  OFX::DoubleParam *m_ChromaValueBlue;
  OFX::DoubleParam *m_ChromaValueCyan;
  OFX::DoubleParam *m_ChromaValueGreen;
  OFX::DoubleParam *m_ChromaValueHueStrengthRgb;
  OFX::DoubleParam *m_ChromaValueHueStrengthCmy;
  OFX::DoubleParam *m_ChromaValueChromaStrength;
  OFX::DoubleParam *m_ChromaValueChromaLimit;
  OFX::BooleanParam *m_ChromaValueZoneEnable;
  OFX::DoubleParam *m_ChromaValueZoneRange;
  OFX::ChoiceParam *m_ChromaValueZoneTarget;

  OFX::DoubleParam *m_VibranceGlobal;
  OFX::DoubleParam *m_VibranceYellow;
  OFX::DoubleParam *m_VibranceRed;
  OFX::DoubleParam *m_VibranceMagenta;
  OFX::DoubleParam *m_VibranceBlue;
  OFX::DoubleParam *m_VibranceCyan;
  OFX::DoubleParam *m_VibranceGreen;
  OFX::DoubleParam *m_VibranceCustom;
  OFX::DoubleParam *m_VibranceCustomHue;
  OFX::DoubleParam *m_VibranceCustomWidth;
  OFX::DoubleParam *m_VibranceHueLinear;
  OFX::BooleanParam *m_VibranceZoneEnable;
  OFX::DoubleParam *m_VibranceZoneRange;
  OFX::ChoiceParam *m_VibranceZoneTarget;

  OFX::DoubleParam *m_HueShiftYellow;
  OFX::DoubleParam *m_HueShiftRed;
  OFX::DoubleParam *m_HueShiftMagenta;
  OFX::DoubleParam *m_HueShiftBlue;
  OFX::DoubleParam *m_HueShiftCyan;
  OFX::DoubleParam *m_HueShiftGreen;
  OFX::DoubleParam *m_HueShiftCustom;
  OFX::DoubleParam *m_HueShiftCustomHue;
  OFX::DoubleParam *m_HueShiftCustomWidth;
  OFX::DoubleParam *m_HueShiftStrength;
  OFX::DoubleParam *m_HueShiftChromaLimit;
  OFX::BooleanParam *m_HueShiftZoneEnable;
  OFX::DoubleParam *m_HueShiftZoneRange;
  OFX::ChoiceParam *m_HueShiftZoneTarget;

  OFX::DoubleParam *m_CrossTalkYellowPower;
  OFX::DoubleParam *m_CrossTalkYellowShift0;
  OFX::DoubleParam *m_CrossTalkYellowShift1;
  OFX::DoubleParam *m_CrossTalkYellowScale;
  OFX::DoubleParam *m_CrossTalkRedPower;
  OFX::DoubleParam *m_CrossTalkRedShift0;
  OFX::DoubleParam *m_CrossTalkRedShift1;
  OFX::DoubleParam *m_CrossTalkRedScale;
  OFX::DoubleParam *m_CrossTalkMagentaPower;
  OFX::DoubleParam *m_CrossTalkMagentaShift0;
  OFX::DoubleParam *m_CrossTalkMagentaShift1;
  OFX::DoubleParam *m_CrossTalkMagentaScale;
  OFX::DoubleParam *m_CrossTalkBluePower;
  OFX::DoubleParam *m_CrossTalkBlueShift0;
  OFX::DoubleParam *m_CrossTalkBlueShift1;
  OFX::DoubleParam *m_CrossTalkBlueScale;
  OFX::DoubleParam *m_CrossTalkCyanPower;
  OFX::DoubleParam *m_CrossTalkCyanShift0;
  OFX::DoubleParam *m_CrossTalkCyanShift1;
  OFX::DoubleParam *m_CrossTalkCyanScale;
  OFX::DoubleParam *m_CrossTalkGreenPower;
  OFX::DoubleParam *m_CrossTalkGreenShift0;
  OFX::DoubleParam *m_CrossTalkGreenShift1;
  OFX::DoubleParam *m_CrossTalkGreenScale;
  OFX::DoubleParam *m_CrossTalkCyanCenter;
  OFX::DoubleParam *m_CrossTalkMagentaCenter;
  OFX::DoubleParam *m_CrossTalkYellowCenter;
};

ODTN6ColorPlugin::ODTN6ColorPlugin(OfxImageEffectHandle handle)
    : OFX::ImageEffect(handle) {
  m_DstClip = fetchClip(kOfxImageEffectOutputClipName);
  m_SrcClip = fetchClip(kOfxImageEffectSimpleSourceClipName);

  m_TransferFunction = fetchChoiceParam(kParamTransferFunction);
  m_OutputMix = fetchDoubleParam(kParamOutputMix);

  m_EnablePurity = fetchBooleanParam(kParamEnablePurity);
  m_EnableChromaValue = fetchBooleanParam(kParamEnableChromaValue);
  m_EnableVibrance = fetchBooleanParam(kParamEnableVibrance);
  m_EnableHueShift = fetchBooleanParam(kParamEnableHueShift);
  m_EnableCrossTalk = fetchBooleanParam(kParamEnableCrossTalk);

  m_PurityMix = fetchDoubleParam(kParamPurityMix);
  m_ChromaValueMix = fetchDoubleParam(kParamChromaValueMix);
  m_VibranceMix = fetchDoubleParam(kParamVibranceMix);
  m_HueShiftMix = fetchDoubleParam(kParamHueShiftMix);
  m_CrossTalkMix = fetchDoubleParam(kParamCrossTalkMix);

  m_PurityRed = fetchDoubleParam(kParamPurityRed);
  m_PurityRedStrength = fetchDoubleParam(kParamPurityRedStrength);
  m_PurityGreen = fetchDoubleParam(kParamPurityGreen);
  m_PurityGreenStrength = fetchDoubleParam(kParamPurityGreenStrength);
  m_PurityBlue = fetchDoubleParam(kParamPurityBlue);
  m_PurityBlueStrength = fetchDoubleParam(kParamPurityBlueStrength);
  m_PurityCyan = fetchDoubleParam(kParamPurityCyan);
  m_PurityCyanStrength = fetchDoubleParam(kParamPurityCyanStrength);
  m_PurityMagenta = fetchDoubleParam(kParamPurityMagenta);
  m_PurityMagentaStrength = fetchDoubleParam(kParamPurityMagentaStrength);
  m_PurityYellow = fetchDoubleParam(kParamPurityYellow);
  m_PurityYellowStrength = fetchDoubleParam(kParamPurityYellowStrength);

  m_ChromaValueYellow = fetchDoubleParam(kParamChromaValueYellow);
  m_ChromaValueRed = fetchDoubleParam(kParamChromaValueRed);
  m_ChromaValueMagenta = fetchDoubleParam(kParamChromaValueMagenta);
  m_ChromaValueBlue = fetchDoubleParam(kParamChromaValueBlue);
  m_ChromaValueCyan = fetchDoubleParam(kParamChromaValueCyan);
  m_ChromaValueGreen = fetchDoubleParam(kParamChromaValueGreen);
  m_ChromaValueHueStrengthRgb =
      fetchDoubleParam(kParamChromaValueHueStrengthRgb);
  m_ChromaValueHueStrengthCmy =
      fetchDoubleParam(kParamChromaValueHueStrengthCmy);
  m_ChromaValueChromaStrength =
      fetchDoubleParam(kParamChromaValueChromaStrength);
  m_ChromaValueChromaLimit = fetchDoubleParam(kParamChromaValueChromaLimit);
  m_ChromaValueZoneEnable =
      fetchBooleanParam(kParamChromaValueZoneEnable);
  m_ChromaValueZoneRange = fetchDoubleParam(kParamChromaValueZoneRange);
  m_ChromaValueZoneTarget = fetchChoiceParam(kParamChromaValueZoneTarget);

  m_VibranceGlobal = fetchDoubleParam(kParamVibranceGlobal);
  m_VibranceYellow = fetchDoubleParam(kParamVibranceYellow);
  m_VibranceRed = fetchDoubleParam(kParamVibranceRed);
  m_VibranceMagenta = fetchDoubleParam(kParamVibranceMagenta);
  m_VibranceBlue = fetchDoubleParam(kParamVibranceBlue);
  m_VibranceCyan = fetchDoubleParam(kParamVibranceCyan);
  m_VibranceGreen = fetchDoubleParam(kParamVibranceGreen);
  m_VibranceCustom = fetchDoubleParam(kParamVibranceCustom);
  m_VibranceCustomHue = fetchDoubleParam(kParamVibranceCustomHue);
  m_VibranceCustomWidth = fetchDoubleParam(kParamVibranceCustomWidth);
  m_VibranceHueLinear = fetchDoubleParam(kParamVibranceHueLinear);
  m_VibranceZoneEnable = fetchBooleanParam(kParamVibranceZoneEnable);
  m_VibranceZoneRange = fetchDoubleParam(kParamVibranceZoneRange);
  m_VibranceZoneTarget = fetchChoiceParam(kParamVibranceZoneTarget);

  m_HueShiftYellow = fetchDoubleParam(kParamHueShiftYellow);
  m_HueShiftRed = fetchDoubleParam(kParamHueShiftRed);
  m_HueShiftMagenta = fetchDoubleParam(kParamHueShiftMagenta);
  m_HueShiftBlue = fetchDoubleParam(kParamHueShiftBlue);
  m_HueShiftCyan = fetchDoubleParam(kParamHueShiftCyan);
  m_HueShiftGreen = fetchDoubleParam(kParamHueShiftGreen);
  m_HueShiftCustom = fetchDoubleParam(kParamHueShiftCustom);
  m_HueShiftCustomHue = fetchDoubleParam(kParamHueShiftCustomHue);
  m_HueShiftCustomWidth = fetchDoubleParam(kParamHueShiftCustomWidth);
  m_HueShiftStrength = fetchDoubleParam(kParamHueShiftStrength);
  m_HueShiftChromaLimit = fetchDoubleParam(kParamHueShiftChromaLimit);
  m_HueShiftZoneEnable = fetchBooleanParam(kParamHueShiftZoneEnable);
  m_HueShiftZoneRange = fetchDoubleParam(kParamHueShiftZoneRange);
  m_HueShiftZoneTarget = fetchChoiceParam(kParamHueShiftZoneTarget);

  m_CrossTalkYellowPower = fetchDoubleParam(kParamCrossTalkYellowPower);
  m_CrossTalkYellowShift0 = fetchDoubleParam(kParamCrossTalkYellowShift0);
  m_CrossTalkYellowShift1 = fetchDoubleParam(kParamCrossTalkYellowShift1);
  m_CrossTalkYellowScale = fetchDoubleParam(kParamCrossTalkYellowScale);
  m_CrossTalkRedPower = fetchDoubleParam(kParamCrossTalkRedPower);
  m_CrossTalkRedShift0 = fetchDoubleParam(kParamCrossTalkRedShift0);
  m_CrossTalkRedShift1 = fetchDoubleParam(kParamCrossTalkRedShift1);
  m_CrossTalkRedScale = fetchDoubleParam(kParamCrossTalkRedScale);
  m_CrossTalkMagentaPower = fetchDoubleParam(kParamCrossTalkMagentaPower);
  m_CrossTalkMagentaShift0 = fetchDoubleParam(kParamCrossTalkMagentaShift0);
  m_CrossTalkMagentaShift1 = fetchDoubleParam(kParamCrossTalkMagentaShift1);
  m_CrossTalkMagentaScale = fetchDoubleParam(kParamCrossTalkMagentaScale);
  m_CrossTalkBluePower = fetchDoubleParam(kParamCrossTalkBluePower);
  m_CrossTalkBlueShift0 = fetchDoubleParam(kParamCrossTalkBlueShift0);
  m_CrossTalkBlueShift1 = fetchDoubleParam(kParamCrossTalkBlueShift1);
  m_CrossTalkBlueScale = fetchDoubleParam(kParamCrossTalkBlueScale);
  m_CrossTalkCyanPower = fetchDoubleParam(kParamCrossTalkCyanPower);
  m_CrossTalkCyanShift0 = fetchDoubleParam(kParamCrossTalkCyanShift0);
  m_CrossTalkCyanShift1 = fetchDoubleParam(kParamCrossTalkCyanShift1);
  m_CrossTalkCyanScale = fetchDoubleParam(kParamCrossTalkCyanScale);
  m_CrossTalkGreenPower = fetchDoubleParam(kParamCrossTalkGreenPower);
  m_CrossTalkGreenShift0 = fetchDoubleParam(kParamCrossTalkGreenShift0);
  m_CrossTalkGreenShift1 = fetchDoubleParam(kParamCrossTalkGreenShift1);
  m_CrossTalkGreenScale = fetchDoubleParam(kParamCrossTalkGreenScale);
  m_CrossTalkCyanCenter = fetchDoubleParam(kParamCrossTalkCyanCenter);
  m_CrossTalkMagentaCenter = fetchDoubleParam(kParamCrossTalkMagentaCenter);
  m_CrossTalkYellowCenter = fetchDoubleParam(kParamCrossTalkYellowCenter);
}

ODTN6ColorParams ODTN6ColorPlugin::getActiveParams(double time) {
  ODTN6ColorParams p;
  std::memset(&p, 0, sizeof(p));

  m_TransferFunction->getValueAtTime(time, p.transferFunction);
  p.outputMix = static_cast<float>(m_OutputMix->getValueAtTime(time));

  p.enablePurity = m_EnablePurity->getValueAtTime(time) ? 1 : 0;
  p.enableChromaValue = m_EnableChromaValue->getValueAtTime(time) ? 1 : 0;
  p.enableVibrance = m_EnableVibrance->getValueAtTime(time) ? 1 : 0;
  p.enableHueShift = m_EnableHueShift->getValueAtTime(time) ? 1 : 0;
  p.enableCrossTalk = m_EnableCrossTalk->getValueAtTime(time) ? 1 : 0;

  p.purityMix = static_cast<float>(m_PurityMix->getValueAtTime(time));
  p.chromaValueMix =
      static_cast<float>(m_ChromaValueMix->getValueAtTime(time));
  p.vibranceMix = static_cast<float>(m_VibranceMix->getValueAtTime(time));
  p.hueShiftMix = static_cast<float>(m_HueShiftMix->getValueAtTime(time));
  p.crossTalkMix = static_cast<float>(m_CrossTalkMix->getValueAtTime(time));

#define GET_DOUBLE(member, param)                                             \
  p.member = static_cast<float>(param->getValueAtTime(time))

  GET_DOUBLE(purityRed, m_PurityRed);
  GET_DOUBLE(purityRedStrength, m_PurityRedStrength);
  GET_DOUBLE(purityGreen, m_PurityGreen);
  GET_DOUBLE(purityGreenStrength, m_PurityGreenStrength);
  GET_DOUBLE(purityBlue, m_PurityBlue);
  GET_DOUBLE(purityBlueStrength, m_PurityBlueStrength);
  GET_DOUBLE(purityCyan, m_PurityCyan);
  GET_DOUBLE(purityCyanStrength, m_PurityCyanStrength);
  GET_DOUBLE(purityMagenta, m_PurityMagenta);
  GET_DOUBLE(purityMagentaStrength, m_PurityMagentaStrength);
  GET_DOUBLE(purityYellow, m_PurityYellow);
  GET_DOUBLE(purityYellowStrength, m_PurityYellowStrength);

  GET_DOUBLE(chromaValueYellow, m_ChromaValueYellow);
  GET_DOUBLE(chromaValueRed, m_ChromaValueRed);
  GET_DOUBLE(chromaValueMagenta, m_ChromaValueMagenta);
  GET_DOUBLE(chromaValueBlue, m_ChromaValueBlue);
  GET_DOUBLE(chromaValueCyan, m_ChromaValueCyan);
  GET_DOUBLE(chromaValueGreen, m_ChromaValueGreen);
  GET_DOUBLE(chromaValueHueStrengthRgb, m_ChromaValueHueStrengthRgb);
  GET_DOUBLE(chromaValueHueStrengthCmy, m_ChromaValueHueStrengthCmy);
  GET_DOUBLE(chromaValueChromaStrength, m_ChromaValueChromaStrength);
  GET_DOUBLE(chromaValueChromaLimit, m_ChromaValueChromaLimit);
  p.chromaValueZoneEnable =
      m_ChromaValueZoneEnable->getValueAtTime(time) ? 1 : 0;
  GET_DOUBLE(chromaValueZoneRange, m_ChromaValueZoneRange);
  m_ChromaValueZoneTarget->getValueAtTime(time, p.chromaValueZoneTarget);

  GET_DOUBLE(vibranceGlobal, m_VibranceGlobal);
  GET_DOUBLE(vibranceYellow, m_VibranceYellow);
  GET_DOUBLE(vibranceRed, m_VibranceRed);
  GET_DOUBLE(vibranceMagenta, m_VibranceMagenta);
  GET_DOUBLE(vibranceBlue, m_VibranceBlue);
  GET_DOUBLE(vibranceCyan, m_VibranceCyan);
  GET_DOUBLE(vibranceGreen, m_VibranceGreen);
  GET_DOUBLE(vibranceCustom, m_VibranceCustom);
  GET_DOUBLE(vibranceCustomHue, m_VibranceCustomHue);
  GET_DOUBLE(vibranceCustomWidth, m_VibranceCustomWidth);
  GET_DOUBLE(vibranceHueLinear, m_VibranceHueLinear);
  p.vibranceZoneEnable = m_VibranceZoneEnable->getValueAtTime(time) ? 1 : 0;
  GET_DOUBLE(vibranceZoneRange, m_VibranceZoneRange);
  m_VibranceZoneTarget->getValueAtTime(time, p.vibranceZoneTarget);

  GET_DOUBLE(hueShiftYellow, m_HueShiftYellow);
  GET_DOUBLE(hueShiftRed, m_HueShiftRed);
  GET_DOUBLE(hueShiftMagenta, m_HueShiftMagenta);
  GET_DOUBLE(hueShiftBlue, m_HueShiftBlue);
  GET_DOUBLE(hueShiftCyan, m_HueShiftCyan);
  GET_DOUBLE(hueShiftGreen, m_HueShiftGreen);
  GET_DOUBLE(hueShiftCustom, m_HueShiftCustom);
  GET_DOUBLE(hueShiftCustomHue, m_HueShiftCustomHue);
  GET_DOUBLE(hueShiftCustomWidth, m_HueShiftCustomWidth);
  GET_DOUBLE(hueShiftStrength, m_HueShiftStrength);
  GET_DOUBLE(hueShiftChromaLimit, m_HueShiftChromaLimit);
  p.hueShiftZoneEnable = m_HueShiftZoneEnable->getValueAtTime(time) ? 1 : 0;
  GET_DOUBLE(hueShiftZoneRange, m_HueShiftZoneRange);
  m_HueShiftZoneTarget->getValueAtTime(time, p.hueShiftZoneTarget);

  GET_DOUBLE(crossTalkYellowPower, m_CrossTalkYellowPower);
  GET_DOUBLE(crossTalkYellowShift0, m_CrossTalkYellowShift0);
  GET_DOUBLE(crossTalkYellowShift1, m_CrossTalkYellowShift1);
  GET_DOUBLE(crossTalkYellowScale, m_CrossTalkYellowScale);
  GET_DOUBLE(crossTalkRedPower, m_CrossTalkRedPower);
  GET_DOUBLE(crossTalkRedShift0, m_CrossTalkRedShift0);
  GET_DOUBLE(crossTalkRedShift1, m_CrossTalkRedShift1);
  GET_DOUBLE(crossTalkRedScale, m_CrossTalkRedScale);
  GET_DOUBLE(crossTalkMagentaPower, m_CrossTalkMagentaPower);
  GET_DOUBLE(crossTalkMagentaShift0, m_CrossTalkMagentaShift0);
  GET_DOUBLE(crossTalkMagentaShift1, m_CrossTalkMagentaShift1);
  GET_DOUBLE(crossTalkMagentaScale, m_CrossTalkMagentaScale);
  GET_DOUBLE(crossTalkBluePower, m_CrossTalkBluePower);
  GET_DOUBLE(crossTalkBlueShift0, m_CrossTalkBlueShift0);
  GET_DOUBLE(crossTalkBlueShift1, m_CrossTalkBlueShift1);
  GET_DOUBLE(crossTalkBlueScale, m_CrossTalkBlueScale);
  GET_DOUBLE(crossTalkCyanPower, m_CrossTalkCyanPower);
  GET_DOUBLE(crossTalkCyanShift0, m_CrossTalkCyanShift0);
  GET_DOUBLE(crossTalkCyanShift1, m_CrossTalkCyanShift1);
  GET_DOUBLE(crossTalkCyanScale, m_CrossTalkCyanScale);
  GET_DOUBLE(crossTalkGreenPower, m_CrossTalkGreenPower);
  GET_DOUBLE(crossTalkGreenShift0, m_CrossTalkGreenShift0);
  GET_DOUBLE(crossTalkGreenShift1, m_CrossTalkGreenShift1);
  GET_DOUBLE(crossTalkGreenScale, m_CrossTalkGreenScale);
  GET_DOUBLE(crossTalkCyanCenter, m_CrossTalkCyanCenter);
  GET_DOUBLE(crossTalkMagentaCenter, m_CrossTalkMagentaCenter);
  GET_DOUBLE(crossTalkYellowCenter, m_CrossTalkYellowCenter);

#undef GET_DOUBLE
  return p;
}

void ODTN6ColorPlugin::changedParam(const OFX::InstanceChangedArgs &,
                                    const std::string &paramName) {
  if (paramName == kParamAboutHelp) {
    openExternalUrl(kAboutHelpUrl);
  } else if (paramName == kParamAppMCNexus) {
    openMCNexusApp();
  }
}

void ODTN6ColorPlugin::render(const OFX::RenderArguments &args) {
  if ((m_DstClip->getPixelDepth() == OFX::eBitDepthFloat) &&
      (m_DstClip->getPixelComponents() == OFX::ePixelComponentRGBA)) {
    ODTN6ColorProcessor processor(*this);
    setupAndProcess(processor, args);
  } else {
    OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
  }
}

bool ODTN6ColorPlugin::isIdentity(const OFX::IsIdentityArguments &args,
                                  OFX::Clip *&identityClip,
                                  double &identityTime) {
  const ODTN6ColorParams p = getActiveParams(args.time);
  if (!odtn6color::hasActiveN6ColorEffect(p)) {
    identityClip = m_SrcClip;
    identityTime = args.time;
    return true;
  }
  return false;
}

void ODTN6ColorPlugin::setupAndProcess(ODTN6ColorProcessor &processor,
                                       const OFX::RenderArguments &args) {
  std::unique_ptr<OFX::Image> dst(m_DstClip->fetchImage(args.time));
  std::unique_ptr<OFX::Image> src(m_SrcClip->fetchImage(args.time));

  if (!dst || !src) {
    OFX::throwSuiteStatusException(kOfxStatFailed);
  }
  if ((src->getPixelDepth() != dst->getPixelDepth()) ||
      (src->getPixelComponents() != dst->getPixelComponents())) {
    OFX::throwSuiteStatusException(kOfxStatErrValue);
  }

  processor.setDstImg(dst.get());
  processor.setSrcImg(src.get());
  processor.setGPURenderArgs(args);
  processor.setRenderWindow(args.renderWindow);
  processor.setParams(getActiveParams(args.time));
  processor.process();
}

ODTN6ColorFactory::ODTN6ColorFactory()
    : OFX::PluginFactoryHelper<ODTN6ColorFactory>(
          kPluginIdentifier, kPluginVersionMajor, kPluginVersionMinor) {}

static OFX::DoubleParamDescriptor *
defineDouble(OFX::ImageEffectDescriptor &desc, const char *name,
             const char *label, double def, double min, double max,
             double displayMin, double displayMax, OFX::GroupParamDescriptor &grp,
             OFX::PageParamDescriptor &page) {
  OFX::DoubleParamDescriptor *param = desc.defineDoubleParam(name);
  param->setLabels(label, label, label);
  param->setDefault(def);
  param->setRange(min, max);
  param->setDisplayRange(displayMin, displayMax);
  param->setIncrement(0.01);
  param->setDoubleType(OFX::eDoubleTypePlain);
  param->setParent(grp);
  page.addChild(*param);
  return param;
}

static OFX::BooleanParamDescriptor *
defineBool(OFX::ImageEffectDescriptor &desc, const char *name,
           const char *label, bool def, OFX::GroupParamDescriptor &grp,
           OFX::PageParamDescriptor &page) {
  OFX::BooleanParamDescriptor *param = desc.defineBooleanParam(name);
  param->setLabels(label, label, label);
  param->setDefault(def);
  param->setParent(grp);
  page.addChild(*param);
  return param;
}

static OFX::ChoiceParamDescriptor *
defineZoneTarget(OFX::ImageEffectDescriptor &desc, const char *name,
                 int def, OFX::GroupParamDescriptor &grp,
                 OFX::PageParamDescriptor &page) {
  OFX::ChoiceParamDescriptor *param = desc.defineChoiceParam(name);
  param->setLabels("Zone Target", "Zone Target", "Zone Target");
  param->appendOption("Low");
  param->appendOption("High");
  param->setDefault(def);
  param->setParent(grp);
  page.addChild(*param);
  return param;
}

void ODTN6ColorFactory::describe(OFX::ImageEffectDescriptor &desc) {
  desc.setLabels(kPluginNameLabel, kPluginNameLabel, kPluginNameLabel);
  desc.setPluginGrouping(kPluginGrouping);
  desc.setPluginDescription(kPluginDescription);

  desc.addSupportedContext(OFX::eContextFilter);
  desc.addSupportedContext(OFX::eContextGeneral);
  desc.addSupportedBitDepth(OFX::eBitDepthFloat);

  desc.setSingleInstance(false);
  desc.setHostFrameThreading(false);
  desc.setSupportsMultiResolution(kSupportsMultiResolution);
  desc.setSupportsTiles(kSupportsTiles);
  desc.setTemporalClipAccess(false);
  desc.setRenderTwiceAlways(false);
  desc.setSupportsMultipleClipPARs(kSupportsMultipleClipPARs);

#ifdef __APPLE__
  desc.setSupportsMetalRender(true);
#else
  desc.setSupportsCudaRender(true);
  desc.setSupportsCudaStream(true);
#endif
}

void ODTN6ColorFactory::describeInContext(OFX::ImageEffectDescriptor &desc,
                                          OFX::ContextEnum) {
  OFX::ClipDescriptor *srcClip =
      desc.defineClip(kOfxImageEffectSimpleSourceClipName);
  srcClip->addSupportedComponent(OFX::ePixelComponentRGBA);
  srcClip->setTemporalClipAccess(false);
  srcClip->setSupportsTiles(kSupportsTiles);
  srcClip->setIsMask(false);

  OFX::ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
  dstClip->addSupportedComponent(OFX::ePixelComponentRGBA);
  dstClip->setSupportsTiles(kSupportsTiles);

  OFX::PageParamDescriptor *page = desc.definePageParam("Controls");

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpSetup");
    grp->setLabels("Setup", "Setup", "Setup");
    grp->setOpen(true);
    page->addChild(*grp);

    OFX::ChoiceParamDescriptor *tf =
        desc.defineChoiceParam(kParamTransferFunction);
    tf->setLabels("Transfer Function", "Transfer Function",
                  "Transfer Function");
    tf->appendOption("Linear");
    tf->appendOption("Davinci Intermediate");
    tf->appendOption("ACEScct");
    tf->appendOption("Arri LogC3");
    tf->appendOption("Arri LogC4");
    tf->appendOption("RedLog3G10");
    tf->setDefault(1);
    tf->setParent(*grp);
    page->addChild(*tf);

    defineDouble(desc, kParamOutputMix, "Output Mix", 1.0, 0.0, 1.0, 0.0,
                 1.0, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpPurity");
    grp->setLabels("Purity", "Purity", "Purity");
    grp->setOpen(true);
    page->addChild(*grp);

    defineBool(desc, kParamEnablePurity, "Enable", true, *grp, *page);
    defineDouble(desc, kParamPurityMix, "Mix", 1.0, 0.0, 1.0, 0.0, 1.0,
                 *grp, *page);
    defineDouble(desc, kParamPurityRed, "Red Purity", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamPurityRedStrength, "Red Strength", 2.0, 0.0, 4.0,
                 0.0, 4.0, *grp, *page);
    defineDouble(desc, kParamPurityGreen, "Green Purity", 0.0, -1.0, 1.0,
                 -1.0, 1.0, *grp, *page);
    defineDouble(desc, kParamPurityGreenStrength, "Green Strength", 2.0, 0.0,
                 4.0, 0.0, 4.0, *grp, *page);
    defineDouble(desc, kParamPurityBlue, "Blue Purity", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamPurityBlueStrength, "Blue Strength", 2.0, 0.0,
                 4.0, 0.0, 4.0, *grp, *page);
    defineDouble(desc, kParamPurityCyan, "Cyan Purity", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamPurityCyanStrength, "Cyan Strength", 2.0, 0.0,
                 4.0, 0.0, 4.0, *grp, *page);
    defineDouble(desc, kParamPurityMagenta, "Magenta Purity", 0.0, -1.0, 1.0,
                 -1.0, 1.0, *grp, *page);
    defineDouble(desc, kParamPurityMagentaStrength, "Magenta Strength", 2.0,
                 0.0, 4.0, 0.0, 4.0, *grp, *page);
    defineDouble(desc, kParamPurityYellow, "Yellow Purity", 0.0, -1.0, 1.0,
                 -1.0, 1.0, *grp, *page);
    defineDouble(desc, kParamPurityYellowStrength, "Yellow Strength", 2.0,
                 0.0, 4.0, 0.0, 4.0, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpChromaValue");
    grp->setLabels("Chroma Value", "Chroma Value", "Chroma Value");
    grp->setOpen(true);
    page->addChild(*grp);

    defineBool(desc, kParamEnableChromaValue, "Enable", true, *grp, *page);
    defineDouble(desc, kParamChromaValueMix, "Mix", 1.0, 0.0, 1.0, 0.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamChromaValueYellow, "Yellow", 0.0, -4.0, 4.0,
                 -4.0, 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueRed, "Red", 0.0, -4.0, 4.0, -4.0,
                 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueMagenta, "Magenta", 0.0, -4.0, 4.0,
                 -4.0, 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueBlue, "Blue", 0.0, -4.0, 4.0, -4.0,
                 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueCyan, "Cyan", 0.0, -4.0, 4.0, -4.0,
                 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueGreen, "Green", 0.0, -4.0, 4.0,
                 -4.0, 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueHueStrengthRgb, "Hue Strength RGB",
                 2.0, 1.0, 4.0, 1.0, 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueHueStrengthCmy, "Hue Strength CMY",
                 2.0, 1.0, 4.0, 1.0, 4.0, *grp, *page);
    defineDouble(desc, kParamChromaValueChromaStrength, "Chroma Strength", 0.5,
                 0.0, 1.0, 0.0, 1.0, *grp, *page);
    defineDouble(desc, kParamChromaValueChromaLimit, "Chroma Limit", 0.0, 0.0,
                 1.0, 0.0, 1.0, *grp, *page);
    defineBool(desc, kParamChromaValueZoneEnable, "Zone Enable", false, *grp,
               *page);
    defineDouble(desc, kParamChromaValueZoneRange, "Zone Range", 0.0, -4.0,
                 4.0, -4.0, 4.0, *grp, *page);
    defineZoneTarget(desc, kParamChromaValueZoneTarget, 1, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpVibrance");
    grp->setLabels("Vibrance", "Vibrance", "Vibrance");
    grp->setOpen(true);
    page->addChild(*grp);

    defineBool(desc, kParamEnableVibrance, "Enable", true, *grp, *page);
    defineDouble(desc, kParamVibranceMix, "Mix", 1.0, 0.0, 1.0, 0.0, 1.0,
                 *grp, *page);
    defineDouble(desc, kParamVibranceGlobal, "Global", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceYellow, "Yellow", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceRed, "Red", 0.0, -1.0, 1.0, -1.0, 1.0,
                 *grp, *page);
    defineDouble(desc, kParamVibranceMagenta, "Magenta", 0.0, -1.0, 1.0,
                 -1.0, 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceBlue, "Blue", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceCyan, "Cyan", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceGreen, "Green", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceCustom, "Custom", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamVibranceCustomHue, "Custom Hue", 100.0, 0.0,
                 360.0, 0.0, 360.0, *grp, *page);
    defineDouble(desc, kParamVibranceCustomWidth, "Custom Width", 0.3, 0.0,
                 2.0, 0.0, 2.0, *grp, *page);
    defineDouble(desc, kParamVibranceHueLinear, "Hue Linear", 0.5, 0.0, 1.0,
                 0.0, 1.0, *grp, *page);
    defineBool(desc, kParamVibranceZoneEnable, "Zone Enable", false, *grp,
               *page);
    defineDouble(desc, kParamVibranceZoneRange, "Zone Range", 0.0, -4.0, 4.0,
                 -4.0, 4.0, *grp, *page);
    defineZoneTarget(desc, kParamVibranceZoneTarget, 0, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpHueShift");
    grp->setLabels("Hue Shift", "Hue Shift", "Hue Shift");
    grp->setOpen(true);
    page->addChild(*grp);

    defineBool(desc, kParamEnableHueShift, "Enable", true, *grp, *page);
    defineDouble(desc, kParamHueShiftMix, "Mix", 1.0, 0.0, 1.0, 0.0, 1.0,
                 *grp, *page);
    defineDouble(desc, kParamHueShiftYellow, "Yellow", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftRed, "Red", 0.0, -1.0, 1.0, -1.0, 1.0,
                 *grp, *page);
    defineDouble(desc, kParamHueShiftMagenta, "Magenta", 0.0, -1.0, 1.0,
                 -1.0, 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftBlue, "Blue", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftCyan, "Cyan", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftGreen, "Green", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftCustom, "Custom", 0.0, -1.0, 1.0, -1.0,
                 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftCustomHue, "Custom Hue", 100.0, 0.0,
                 360.0, 0.0, 360.0, *grp, *page);
    defineDouble(desc, kParamHueShiftCustomWidth, "Custom Width", 0.3, 0.0,
                 2.0, 0.0, 2.0, *grp, *page);
    defineDouble(desc, kParamHueShiftStrength, "Strength", 0.33, 0.1, 1.0,
                 0.1, 1.0, *grp, *page);
    defineDouble(desc, kParamHueShiftChromaLimit, "Chroma Limit", 0.33, 0.0,
                 1.0, 0.0, 1.0, *grp, *page);
    defineBool(desc, kParamHueShiftZoneEnable, "Zone Enable", false, *grp,
               *page);
    defineDouble(desc, kParamHueShiftZoneRange, "Zone Range", 0.0, -4.0, 4.0,
                 -4.0, 4.0, *grp, *page);
    defineZoneTarget(desc, kParamHueShiftZoneTarget, 1, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpCrossTalk");
    grp->setLabels("CrossTalk", "CrossTalk", "CrossTalk");
    grp->setOpen(false);
    page->addChild(*grp);

    defineBool(desc, kParamEnableCrossTalk, "Enable", true, *grp, *page);
    defineDouble(desc, kParamCrossTalkMix, "Mix", 1.0, 0.0, 1.0, 0.0, 1.0,
                 *grp, *page);

#define CROSS_TALK_HUE(prefix, label, power, shift0, shift1, scale)           \
  defineDouble(desc, power, label " Power", 1.0, 0.0, 1.0, 0.0, 1.0, *grp,   \
               *page);                                                       \
  defineDouble(desc, shift0, label " Shift 0", 0.0, -1.0, 1.0, -1.0, 1.0,    \
               *grp, *page);                                                 \
  defineDouble(desc, shift1, label " Shift 1", 0.0, -1.0, 1.0, -1.0, 1.0,    \
               *grp, *page);                                                 \
  defineDouble(desc, scale, label " Scale", 0.0, -1.0, 1.0, -1.0, 1.0,       \
               *grp, *page)

    CROSS_TALK_HUE(yellow, "Yellow", kParamCrossTalkYellowPower,
                   kParamCrossTalkYellowShift0, kParamCrossTalkYellowShift1,
                   kParamCrossTalkYellowScale);
    CROSS_TALK_HUE(red, "Red", kParamCrossTalkRedPower,
                   kParamCrossTalkRedShift0, kParamCrossTalkRedShift1,
                   kParamCrossTalkRedScale);
    CROSS_TALK_HUE(magenta, "Magenta", kParamCrossTalkMagentaPower,
                   kParamCrossTalkMagentaShift0, kParamCrossTalkMagentaShift1,
                   kParamCrossTalkMagentaScale);
    CROSS_TALK_HUE(blue, "Blue", kParamCrossTalkBluePower,
                   kParamCrossTalkBlueShift0, kParamCrossTalkBlueShift1,
                   kParamCrossTalkBlueScale);
    CROSS_TALK_HUE(cyan, "Cyan", kParamCrossTalkCyanPower,
                   kParamCrossTalkCyanShift0, kParamCrossTalkCyanShift1,
                   kParamCrossTalkCyanScale);
    CROSS_TALK_HUE(green, "Green", kParamCrossTalkGreenPower,
                   kParamCrossTalkGreenShift0, kParamCrossTalkGreenShift1,
                   kParamCrossTalkGreenScale);

#undef CROSS_TALK_HUE

    defineDouble(desc, kParamCrossTalkCyanCenter, "Cyan Center", 0.25, -0.8,
                 0.8, -0.8, 0.8, *grp, *page);
    defineDouble(desc, kParamCrossTalkMagentaCenter, "Magenta Center", 0.0,
                 -0.8, 0.8, -0.8, 0.8, *grp, *page);
    defineDouble(desc, kParamCrossTalkYellowCenter, "Yellow Center", -0.25,
                 -0.8, 0.8, -0.8, 0.8, *grp, *page);
  }

  {
    OFX::GroupParamDescriptor *grp = desc.defineGroupParam("grpSupport");
    grp->setLabels("Support", "Support", "Support");
    grp->setOpen(false);
    page->addChild(*grp);

    OFX::PushButtonParamDescriptor *aboutHelp =
        desc.definePushButtonParam(kParamAboutHelp);
    aboutHelp->setLabels("About and Help", "About and Help", "About and Help");
    aboutHelp->setParent(*grp);
    page->addChild(*aboutHelp);

    OFX::PushButtonParamDescriptor *appMCNexus =
        desc.definePushButtonParam(kParamAppMCNexus);
    appMCNexus->setLabels("App MCNexus", "App MCNexus", "App MCNexus");
#if !defined(__APPLE__) && !defined(_WIN32)
    appMCNexus->setEnabled(false);
#endif
    appMCNexus->setParent(*grp);
    page->addChild(*appMCNexus);
  }
}

OFX::ImageEffect *ODTN6ColorFactory::createInstance(OfxImageEffectHandle handle,
                                                    OFX::ContextEnum) {
  return new ODTN6ColorPlugin(handle);
}

void OFX::Plugin::getPluginIDs(OFX::PluginFactoryArray &factoryArray) {
  static ODTN6ColorFactory plugin;
  factoryArray.push_back(&plugin);
}
