#ifndef BASEAPP_H_
#define BASEAPP_H_

typedef void (*publishHandler)(char*);

class BaseApp
{
public:
    virtual void onButtonPress(){};
    virtual void onButtonMessage(){};
    virtual void onOwnButtonMessage(){};
};

#endif /* BASEAPP_H_ */
