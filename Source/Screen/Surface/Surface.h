/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once




class Media;

class Surface :
    public BaseItem,
    public MediaTarget
{
public:
    Surface(var params = var());
    virtual ~Surface();

    String objectType;
    var objectData;

    bool shouldUpdateVertices;

    TargetParameter* media;

    ControllableContainer positionningCC;
    Point2DParameter* topLeft;
    Point2DParameter* topRight;
    Point2DParameter* bottomLeft;
    Point2DParameter* bottomRight;

    EnablingControllableContainer bezierCC;
    Point2DParameter* handleBezierTopLeft;
    Point2DParameter* handleBezierTopRight;
    Point2DParameter* handleBezierBottomLeft;
    Point2DParameter* handleBezierBottomRight;
    Point2DParameter* handleBezierLeftTop;
    Point2DParameter* handleBezierLeftBottom;
    Point2DParameter* handleBezierRightTop;
    Point2DParameter* handleBezierRightBottom;

    ControllableContainer adjustmentsCC;
    BoolParameter* showTestPattern;
    TargetParameter* mask;
    BoolParameter* invertMask;

    ControllableContainer formatCC;
    enum FillType {STRETCH, FIT, FILL};
    EnumParameter* fillType;
    enum Ratio { R4_3, R16_9, R16_10, R1, RCUSTOM };
    EnumParameter* ratioList;
    FloatParameter* ratio;
    BoolParameter* considerCrop;

    FloatParameter* softEdgeTop;
    FloatParameter* softEdgeRight;
    FloatParameter* softEdgeBottom;
    FloatParameter* softEdgeLeft;

    FloatParameter* cropTop;
    FloatParameter* cropRight;
    FloatParameter* cropBottom;
    FloatParameter* cropLeft;

    Media* previewMedia;
    Path quadPath;

    // openGL variables
    GLuint vbo;
    GLint posAttrib;
    GLint surfacePosAttrib;
    GLint texAttrib;
    GLint maskAttrib;
    GLuint borderSoftLocation;
    GLuint invertMaskLocation;
    GLuint ratioLocation;
    GLuint ebo;


    void onContainerParameterChangedInternal(Parameter* p);
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

    void updatePath();

    bool isPointInside(Point<float> pos);

    void resetBezierPoints();
    Trigger* resetBezierBtn;

    unsigned int verticesVersion;
    Array<GLfloat> vertices;
    Array<GLuint> verticesElements;
    CriticalSection verticesLock;

    void addToVertices(Point<float> posDisplay, Point<float>itnernalCoord, Vector3D<float> texCoord, Vector3D<float> maskCoord);
    void addLastFourAsQuad();
    void updateVertices();
    void draw(GLuint shaderID);

    Media* getMedia();
    Point<int> getMediaSize();

    bool isUsingMedia(Media* m) override;
    
    Array<Point2DParameter*> getCornerHandles();
    Array<Point2DParameter*> getAllHandles();
    Array<Point2DParameter*> getBezierHandles(Point2DParameter* corner = nullptr);

    String getTypeString() const override { return objectType; }
    static Surface* create(var params) { return new Surface(params); }

    static Point<float> getBeziers(Point<float>a, Point<float>b, Point<float>c, Point<float>d, float r);
    static bool intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect); // should be in another objet
    static Point<float> openGLPoint(Point2DParameter* p);

};


