/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Screen :
    public BaseItem
{
public:
    Screen(var params = var());
    virtual ~Screen();

    String objectType;
    var objectData;
    
    enum OutputType { DISPLAY, SHARED_TEXTURE, NDI };
    EnumParameter* outputType;
    IntParameter* screenID;

    FloatParameter* snapDistance;

    SurfaceManager surfaces;

    Point2DParameter* getClosestHandle(Point<float> pos, float maxDistance = INT32_MAX, Array<Point2DParameter*> excludeHandles = {});
    Point2DParameter* getSnapHandle(Point<float> pos, Point2DParameter* handle);
    Array<Point2DParameter*> getOverlapHandles(Point2DParameter* handle);
    Surface* getSurfaceAt(Point<float> pos);

    String getTypeString() const override { return objectType; }
    static Screen* create(var params) { return new Screen(params); }
};


