/*
  ==============================================================================

    MediaComposition.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaComposition :
    public Media
{
public:
    MediaComposition(var params = var());
    ~MediaComposition();

    Point2DParameter* resolution;
 
    CompositionLayerManager layers;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    
    void renderOpenGL();

    std::shared_ptr<Graphics> myGraphics = nullptr;
    std::shared_ptr<Graphics> workGraphics = nullptr;
    Image workImage;

    void updateImagesSize();
    bool imageNeedRepaint = true;

    
    DECLARE_TYPE("Composition")

    void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

    HashMap<Media*, int> texturesVersions;

};