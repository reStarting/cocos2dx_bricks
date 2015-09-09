#include "HelloWorldScene.h"

#define EDGEMASK  0b00001
#define BALLMASK  0b00010
#define PLATEMASK 0b00100
#define BRICKMASK 0b01000
#define BALL_BIT_PLATE 0b00110
#define BALL_BIT_EDGE  0b00011
#define BALL_BIT_BRICK 0b01010

#define BRICKWIDTH 100
#define BRICKHEIGHT 20


Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setDebugDrawMask(1);
//    scene->getPhysicsWorld()->setSpeed(2);
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();
    layer->isMoving = false;

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto edgeBox = PhysicsBody::createEdgeBox(visibleSize,PhysicsMaterial(0,1,0));
    edgeBox->setContactTestBitmask(EDGEMASK);
    Node * border = Node::create();
    border->setPhysicsBody(edgeBox);
    border->setPosition(visibleSize/2);
    addChild(border);
    
    
    //创建档板
    mPlate = DrawNode::create();
    mPlate->drawSolidRect(Vec2(0,0), Vec2(100,10), Color4F::GRAY);
    mPlate->setAnchorPoint(Vec2(0.5,0.5));
    mPlate->setPosition(visibleSize.width/2, 40);
    mPlate->setContentSize(Size(100,10));
    auto platePb = PhysicsBody::createBox(Size(100,10),PhysicsMaterial(0,1,0));
    platePb->setGravityEnable(false);
    platePb->setVelocity(Vect(0,0));
    platePb->setDynamic(false);
    platePb->setContactTestBitmask(PLATEMASK);
    mPlate->setPhysicsBody(platePb);
    addChild(mPlate);
    
    //创建小球
    mBall = DrawNode::create();
    mBall->drawSolidCircle(Vec2(15,15), 15, M_2_PI, 500, 1, 1, Color4F::GRAY);
    mBall->setContentSize(Size(30,30));
    mBall->setAnchorPoint(Vec2(0.5,0.5));
    mBall->setPosition(visibleSize.width/2, 65);
    auto ballPb = PhysicsBody::createCircle(15,PhysicsMaterial(0,1,0));
    ballPb->setGravityEnable(false);
    ballPb->setVelocity(Vect(40,400));
    ballPb->setContactTestBitmask(BALLMASK);
    mBall->setPhysicsBody(ballPb);

    addChild(mBall);
    
    //监听触摸事件,实现档板左右移动
    auto moveListeners = EventListenerTouchOneByOne::create();
    moveListeners->onTouchBegan = [this](Touch* t, Event* e){
        isMoving = true;
        startPoint = t->getLocation();
//        log("start=%f", startPoint.x);
        return true;
    };
    moveListeners->onTouchMoved = [this](Touch* t, Event* e){
        if(isMoving)
        {
//            log("start=%f,location=%f",startPoint.x, t->getLocation().x);
            auto movePosition = t->getLocation() - startPoint;
            movePosition.y=0;
            mPlate->runAction(MoveBy::create(1, movePosition));
            startPoint = t->getLocation();
        }
    };
    moveListeners->onTouchEnded = [this](Touch* t, Event* e){
         isMoving = false;
    };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(moveListeners, this);
    
    //碰撞检测
    auto pcListener = EventListenerPhysicsContact::create();
    pcListener->onContactBegin = [this](PhysicsContact& contact){
        auto data = contact.getContactData();
        if(data->points[0].y < 2)
        {
            //游戏结束
            log("over");
        }
        int aBitMask = contact.getShapeA()->getBody()->getContactTestBitmask();
        int bBitMask = contact.getShapeB()->getBody()->getContactTestBitmask();
        auto aNode = contact.getShapeA()->getBody()->getNode();
        auto bNode = contact.getShapeB()->getBody()->getNode();
        switch (aBitMask | bBitMask) {
            case BALL_BIT_EDGE:
                log("墙和球");
                break;
            case BALL_BIT_PLATE:
                log("档板和球");
                break;
            case BALL_BIT_BRICK:
                if(aBitMask == BRICKMASK)
                {
                    this->removeChild(aNode);
                }
                else if(bBitMask == BRICKMASK)
                {
                    this->removeChild(bNode);
                }
                break;
        }
        return true;
    };
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(pcListener, this);
    
    int count = visibleSize.width/BRICKWIDTH;
    int h = visibleSize.height;
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<count-1; j++)
        {
            auto brick = DrawNode::create();
            log("%d,%d",BRICKWIDTH*j, h-BRICKHEIGHT*i);
            brick->drawSolidRect(Vec2(0,0), Vec2(BRICKWIDTH, BRICKHEIGHT), Color4F::BLUE);
            brick->setContentSize(Size(BRICKWIDTH, BRICKHEIGHT));
            brick->setAnchorPoint(Vec2::ZERO);
            brick->setPosition(Vec2(60+(BRICKWIDTH+10)*j, h-(BRICKHEIGHT+8)*(i+2)));
            auto brickPb = PhysicsBody::createBox(Size(BRICKWIDTH,BRICKHEIGHT),PhysicsMaterial(0,1,0));
            brickPb->setGravityEnable(false);
            brickPb->setDynamic(false);
            brickPb->setContactTestBitmask(BRICKMASK);
            brick->setPhysicsBody(brickPb);
            addChild(brick);
        }
    }
    
    
    return true;
}

