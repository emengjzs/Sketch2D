#include "DrawableSprite.h"
#include "util/VisibleRect.h"

USING_NS_CC;

using namespace DollarRecognizer;

#define SCALE 2
#define VEC2_TO_POINT2D(vec2, scale) Point2D(vec2.x/(scale), vec2.y/(scale))
#define NORMALIZE_VEC2_TO_POINT2D(vec2, scale) Point2D((vec2.x-_xMin)/(scale), (vec2.y-_yMin)/(scale))
#define POINT2D_TO_VEC2(p2d) Vec2((p2d).x, (p2d).y)

DrawableSprite::DrawableSprite()
:_xMin(VisibleRect::width())
, _xMax(0)
, _yMin(VisibleRect::height())
, _yMax(0)
, _reference(nullptr)
, _brushColor(Color4F::WHITE)
{}

/*bool DrawableSprite::init()
{
	if (!DrawNode::init())
	{
		return false; 
	}
	
	return true;
}*/

void DrawableSprite::onEnter()
{
	DrawNode::onEnter();
	
	//this->addChild(_target);
}

void DrawableSprite::onExit()
{
	//_target->release();

	//Director::getInstance()->getTextureCache()->removeUnusedTextures();
	DrawNode::onExit();
}

void DrawableSprite::addToPath(Vec2 from, Vec2 to)
{
	//if (!_path.empty()){ _path.pop_back(); }
	this->_path.push_back(from); this->_path.push_back(to);
	_baryCenter = (_baryCenter*(_path.size() / 2 - 1) + (from + to) / 2) / (_path.size() / 2);
	_xMin = MIN(MIN(from.x, to.x), _xMin);
	_xMax = MAX(MAX(from.x, to.x), _xMax);
	_yMin = MIN(MIN(from.y, to.y), _yMin);
	_yMax = MAX(MAX(from.y, to.y), _yMax);
}

void DrawableSprite::redraw()
{
	for (auto p = _path.begin(); p != _path.end(); p+=2)
	{
		this->drawLine(*p, *(p + 1), _brushColor);
	}
}

DrawableSprite* DrawableSprite::createWithMultiStrokeGesture(const MultiStrokeGesture& multiStrokes)
{
	DrawableSprite *pRet = new(std::nothrow) DrawableSprite();
	if (pRet && pRet->init())
	{
		for (auto path = multiStrokes.begin(); path != multiStrokes.end(); path++)
		{
			if (!path->empty())
			{
				auto startLoc = POINT2D_TO_VEC2(*(path->begin()));
				for (auto p = path->begin() + 1; p != path->end(); p++)
				{
					auto endLoc = POINT2D_TO_VEC2(*p);
					pRet->addToPath(startLoc, endLoc);
					startLoc = endLoc;
				}
			}
		}
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

MultiStrokeGesture& DrawableSprite::getMultiStrokeGesture(MultiStrokeGesture& multiStrokes)
{
	Vec2 prev = *(_path.begin());
	Path2D path;
	path.push_back(NORMALIZE_VEC2_TO_POINT2D(prev, SCALE));
	for (auto p = _path.begin(); p != _path.end(); p += 2)
	{
		Vec2 start = *p, end = *(p + 1);
		if (start == prev)
		{
			path.push_back(NORMALIZE_VEC2_TO_POINT2D(prev, SCALE));
		}
		else
		{
			path.push_back(NORMALIZE_VEC2_TO_POINT2D(prev, SCALE));
			multiStrokes.push_back(path);
			path.clear();
			path.push_back(NORMALIZE_VEC2_TO_POINT2D(start, SCALE));
		}

		prev = end;
	}
	multiStrokes.push_back(path);
	return multiStrokes;
}

RecognitionResult DrawableSprite::recognize()
{
	if (!this->_path.empty())
	{
		MultiStrokeGesture multiStrokes;
		getMultiStrokeGesture(multiStrokes);
		RecognitionResult result = _geoRecognizer->Multirecognize(multiStrokes, "normal");
		log("Recognized gesture: %s, Score: %f", result.name.c_str(), result.score);
		return result;
	}
	return RecognitionResult("", 0);
}

Texture2D* DrawableSprite::createTexture()
{
	auto _target = RenderTexture::create(VisibleRect::width(), VisibleRect::height(), Texture2D::PixelFormat::RGBA8888);
	_target->begin();
	this->visit();
	_target->end();
	return _target->getSprite()->getTexture();
}

bool DrawableSprite::containsPoint(const Vec2& point)
{
	bool hasMin = false, hasMax = false;
	for (auto p = _path.begin(); p != _path.end(); p ++)
	{
		hasMin |= p->x <= point.x&&p->y <= point.y;
		hasMax |= p->x >= point.x&&p->y >= point.y;
		if (hasMin&&hasMax)return true;
	}
	return false;
}