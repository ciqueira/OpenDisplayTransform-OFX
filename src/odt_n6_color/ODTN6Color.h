// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.

#pragma once

#include "ofxsImageEffect.h"

class ODTN6ColorFactory : public OFX::PluginFactoryHelper<ODTN6ColorFactory> {
public:
  ODTN6ColorFactory();
  virtual void load() {}
  virtual void unload() {}
  virtual void describe(OFX::ImageEffectDescriptor &desc);
  virtual void describeInContext(OFX::ImageEffectDescriptor &desc,
                                 OFX::ContextEnum context);
  virtual OFX::ImageEffect *createInstance(OfxImageEffectHandle handle,
                                           OFX::ContextEnum context);
};
