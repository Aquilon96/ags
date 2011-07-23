
#define THIS_IS_THE_PLUGIN
#include "agsplugin.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef PSP_VERSION
#include <pspsdk.h>
#include <pspmath.h>
#define sin(x) vfpu_sinf(x)
#endif


#define signum(x) ((x > 0) ? 1 : -1)

const float PI = 3.14159265f;

long int screen_width = 320;
long int screen_height = 200;
long int screen_color_depth = 32;

IAGSEngine* engine;


typedef struct
{
  int view;
  int loop;
  bool is_default;
  BITMAP* bitmap;
} view_t;


typedef struct
{
  float x;
  float y;
  int alpha;
  float speed;
  int max_y;
  int kind_id;
  int drift;
  float drift_speed;
  float drift_offset;
} drop_t;


class Weather
{
  public:
    Weather();
    Weather(bool IsSnow);
    ~Weather();
    
    void Initialize();
    void InitializeParticles();
    
    bool IsActive();
    void Update();
    void UpdateWithDrift();
    
    void SetDriftRange(int min_value, int max_value);
    void SetDriftSpeed(int min_value, int max_value);
    void ChangeAmount(int amount);
    void SetView(int kind_id, int event, int view, int loop);
    void SetDefaultView(int view, int loop);
    void SetTransparency(int min_value, int max_value);
    void SetWindSpeed(int value);
    void SetBaseline(int top, int bottom);
    void SetAmount(int amount);
    void SetFallSpeed(int min_value, int max_value);

  private:
    void ClipToRange(int &variable, int min, int max);
    
    bool mIsSnow;
    
    int mMinDrift;
    int mMaxDrift;
    int mDeltaDrift;
    
    int mMinDriftSpeed;
    int mMaxDriftSpeed;
    int mDeltaDriftSpeed;
    
    int mAmount;
    int mTargetAmount;
        
    int mMinAlpha;
    int mMaxAlpha;
    int mDeltaAlpha;
    
    float mWindSpeed;
    
    int mTopBaseline;
    int mBottomBaseline;
    int mDeltaBaseline;
    
    int mMinFallSpeed;
    int mMaxFallSpeed;
    int mDeltaFallSpeed;
    
    drop_t mParticles[1000];
    view_t mViews[5];
};


Weather::Weather()
{
  mIsSnow = false;
  Initialize();
}


Weather::Weather(bool IsSnow)
{
  mIsSnow = IsSnow;
  Initialize();
}


Weather::~Weather()
{
}


void Weather::Update()
{
  if (mTargetAmount > mAmount)
    mAmount++;
  else if (mTargetAmount < mAmount)
    mAmount--;

  int i;
  for (i = 0; i < mAmount; i++)
  {
    mParticles[i].y += mParticles[i].speed;
    mParticles[i].x += mWindSpeed;
    
    if (mParticles[i].x < 0)
      mParticles[i].x += screen_width;
      
    if (mParticles[i].x > screen_width - 1)
      mParticles[i].x -= screen_width;
    
    if (mParticles[i].y > mParticles[i].max_y)
    {
      mParticles[i].y = -1 * (rand() % 20) + 1;
      mParticles[i].x = rand() % screen_width;
      mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
      mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 40.0f;
      mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
    }
    else
      engine->BlitSpriteTranslucent(mParticles[i].x, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
  }
  
  engine->MarkRegionDirty(0, 0, screen_width, mBottomBaseline);
}


void Weather::UpdateWithDrift()
{
  if (mTargetAmount > mAmount)
    mAmount++;
  else if (mTargetAmount < mAmount)
    mAmount--;

  int i, drift;
  for (i = 0; i < mAmount; i++)
  {
    mParticles[i].y += mParticles[i].speed;
    drift = mParticles[i].drift * sin((float)(mParticles[i].y + mParticles[i].drift_offset) * mParticles[i].drift_speed * 2.0f * PI / 360.0f);

    if (signum(mWindSpeed) == signum(drift))
      mParticles[i].x += mWindSpeed;
    else
      mParticles[i].x += mWindSpeed / 4;
    
    if (mParticles[i].x < 0)
      mParticles[i].x += screen_width;
      
    if (mParticles[i].x > screen_width - 1)
      mParticles[i].x -= screen_width;
    
    if (mParticles[i].y > mParticles[i].max_y)
    {
      mParticles[i].y = -1 * (rand() % 20) + 1;
      mParticles[i].x = rand() % screen_width;
      mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
      mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 40.0f;
      mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
      mParticles[i].drift = rand() % mDeltaDrift + mMinDrift;
      mParticles[i].drift_speed = (rand() % mDeltaDriftSpeed + mMinDriftSpeed) / 40.0f;
    }
    else
      engine->BlitSpriteTranslucent(mParticles[i].x + drift, mParticles[i].y, mViews[mParticles[i].kind_id].bitmap, mParticles[i].alpha);
  }
  
  engine->MarkRegionDirty(0, 0, screen_width, mBottomBaseline);
}


bool Weather::IsActive()
{
  return (mAmount > 0) || (mTargetAmount != mAmount);
}


void Weather::ClipToRange(int &variable, int min, int max)
{
  if (variable < min)
    variable = min;
  
  if (variable > max)
    variable = max;
}


void Weather::Initialize()
{
  SetDriftRange(10, 100);
  SetDriftSpeed(10, 120);  
    
  SetAmount(0);
  SetTransparency(0, 0);  
  SetWindSpeed(0);  
  SetBaseline(0, 200);  
  
  if (mIsSnow)
    SetFallSpeed(10, 70);
  else
    SetFallSpeed(100, 300);
  
  int i;
  for (i = 0; i < 5; i++)
  {
    mViews[i].is_default = true;
    mViews[i].view = 0;
    mViews[i].loop = 0;
    mViews[i].bitmap = NULL;
  }
  
  InitializeParticles();
}


void Weather::InitializeParticles()
{
  memset(mParticles, 0, sizeof(drop_t) * 1000);
  int i;
  for (i = 0; i < 1000; i++)
  {
    mParticles[i].kind_id = rand() % 5;
    mParticles[i].y = 10000; // outside the screen
    mParticles[i].x = rand() % screen_width;
    mParticles[i].alpha = rand() % mDeltaAlpha + mMinAlpha;
    mParticles[i].speed = (float)(rand() % mDeltaFallSpeed + mMinFallSpeed) / 40.0f;
    mParticles[i].max_y = rand() % mDeltaBaseline + mTopBaseline;
    mParticles[i].drift = rand() % mDeltaDrift + mMinDrift;
    mParticles[i].drift_speed = (rand() % mDeltaDriftSpeed + mMinDriftSpeed) / 40.0f;  
    mParticles[i].drift_offset = rand() % 100;
  }
}


void Weather::SetDriftRange(int min_value, int max_value)
{
  ClipToRange(min_value, 0, 100);
  ClipToRange(max_value, 0, 100);

  if (min_value > max_value)
    min_value = max_value;
  
  mMinDrift = min_value / 2;
  mMaxDrift = max_value / 2;
  mDeltaDrift = mMaxDrift - mMinDrift;
  
  if (mDeltaDrift == 0)
    mDeltaDrift = 1;  
}


void Weather::SetDriftSpeed(int min_value, int max_value)
{
  ClipToRange(min_value, 0, 200);
  ClipToRange(max_value, 0, 200);
  
  if (min_value > max_value)
    min_value = max_value;
  
  mMinDriftSpeed = min_value;
  mMaxDriftSpeed = max_value;
  mDeltaDriftSpeed = mMaxDriftSpeed - mMinDriftSpeed;
  
  if (mDeltaDriftSpeed == 0)
    mDeltaDriftSpeed = 1;
}


void Weather::ChangeAmount(int amount)
{
  ClipToRange(amount, 0, 1000);
  
  mTargetAmount = amount;
}


void Weather::SetView(int kind_id, int event, int view, int loop)
{
  AGSViewFrame* view_frame = engine->GetViewFrame(view, loop, 0);
  mViews[kind_id].bitmap = engine->GetSpriteGraphic(view_frame->pic);
  mViews[kind_id].is_default = false;  
  mViews[kind_id].view = view;
  mViews[kind_id].loop = loop;  
}


void Weather::SetDefaultView(int view, int loop)
{
  AGSViewFrame* view_frame = engine->GetViewFrame(view, loop, 0);
  BITMAP* bitmap = engine->GetSpriteGraphic(view_frame->pic);
  
  int i;
  for (i = 0; i < 5; i++)
  {
    if (mViews[i].is_default)
    {
      mViews[i].view = view;
      mViews[i].loop = loop;
      mViews[i].bitmap = bitmap;
    }
  }  
}


void Weather::SetTransparency(int min_value, int max_value)
{
  ClipToRange(min_value, 0, 100);
  ClipToRange(max_value, 0, 100);
  
  if (min_value > max_value)
    min_value = max_value;
    
  mMinAlpha = 255 - floor((float)max_value * 2.55f + 0.5f);
  mMaxAlpha = 255 - floor((float)min_value * 2.55f + 0.5f);
  mDeltaAlpha = mMaxAlpha - mMinAlpha;
  
  if (mDeltaAlpha == 0)
    mDeltaAlpha = 1;
}


void Weather::SetWindSpeed(int value)
{
  ClipToRange(value, -200, 200);
  
  mWindSpeed = (float)value / 20.0f;
}


void Weather::SetBaseline(int top, int bottom)
{
  if (screen_height > 0)
  {
    ClipToRange(top, 0, screen_height);
    ClipToRange(bottom, 0, screen_height);
  }
  
  if (top > bottom)
    top = bottom;    

  mTopBaseline = top;
  mBottomBaseline = bottom;
  mDeltaBaseline = mBottomBaseline - mTopBaseline;
  
  if (mDeltaBaseline == 0)
    mDeltaBaseline = 1;
}


void Weather::SetAmount(int amount)
{
  ClipToRange(amount, 0, 1000);

  mAmount = mTargetAmount = amount;
}


void Weather::SetFallSpeed(int min_value, int max_value)
{
  ClipToRange(min_value, 0, 1000);
  ClipToRange(max_value, 0, 1000);
  
  if (min_value > max_value)
    min_value = max_value;

  mMinFallSpeed = min_value;
  mMaxFallSpeed = max_value;
  mDeltaFallSpeed = mMaxFallSpeed - mMinFallSpeed;
  
  if (mDeltaFallSpeed == 0)
    mDeltaFallSpeed = 1;
}



Weather* rain;
Weather* snow;




// ********************************************
// ************  AGS Interface  ***************
// ********************************************

void srSetWindSpeed(int value)
{
  snow->SetWindSpeed(value);
  rain->SetWindSpeed(value);
}

void srSetBaseline(int top, int bottom)
{
  snow->SetBaseline(top, bottom);
  rain->SetBaseline(top, bottom);
}

void srSetSnowDriftRange(int min_value, int max_value)
{
  snow->SetDriftRange(min_value, max_value);
}

void srSetSnowDriftSpeed(int min_value, int max_value)
{
  snow->SetDriftSpeed(min_value, max_value);
}

void srChangeSnowAmount(int amount)
{
  snow->ChangeAmount(amount);
}

void srSetSnowView(int kind_id, int event, int view, int loop)
{
  snow->SetView(kind_id, event, view, loop);
}

void srSetSnowDefaultView(int view, int loop)
{
  snow->SetDefaultView(view, loop);
}

void srSetSnowTransparency(int min_value, int max_value)
{
  snow->SetTransparency(min_value, max_value);
}

void srSetSnowWindSpeed(int value)
{
  snow->SetWindSpeed(value);
}

void srSetSnowBaseline(int top, int bottom)
{
  snow->SetBaseline(top, bottom);
}

void srSetSnowAmount(int amount)
{
  snow->SetAmount(amount);
}

void srSetSnowFallSpeed(int min_value, int max_value)
{
  snow->SetFallSpeed(min_value, max_value);
}

void srSetRainDriftRange(int min_value, int max_value)
{
  rain->SetDriftRange(min_value, max_value);
}

void srSetRainDriftSpeed(int min_value, int max_value)
{
  rain->SetDriftSpeed(min_value, max_value);
}

void srChangeRainAmount(int amount)
{
  rain->ChangeAmount(amount);
}

void srSetRainView(int kind_id, int event, int view, int loop)
{
  rain->SetView(kind_id, event, view, loop);
}

void srSetRainDefaultView(int view, int loop)
{
  rain->SetDefaultView(view, loop);
}

void srSetRainTransparency(int min_value, int max_value)
{
  rain->SetTransparency(min_value, max_value);
}

void srSetRainWindSpeed(int value)
{
  rain->SetWindSpeed(value);
}

void srSetRainBaseline(int top, int bottom)
{
  rain->SetBaseline(top, bottom);
}

void srSetRainAmount(int amount)
{
  rain->SetAmount(amount);
}

void srSetRainFallSpeed(int min_value, int max_value)
{
  rain->SetFallSpeed(min_value, max_value);
}



void AGS_EngineStartup(IAGSEngine *lpEngine)
{
  engine = lpEngine;
  
  if (engine->version < 13) 
    engine->AbortGame("Engine interface is too old, need newer version of AGS.");

  engine->RegisterScriptFunction("srSetSnowDriftRange", (void*)&srSetSnowDriftRange);
  engine->RegisterScriptFunction("srSetSnowDriftSpeed", (void*)&srSetSnowDriftSpeed);
  engine->RegisterScriptFunction("srSetSnowFallSpeed", (void*)&srSetSnowFallSpeed);
  engine->RegisterScriptFunction("srChangeSnowAmount", (void*)&srChangeSnowAmount);
  engine->RegisterScriptFunction("srSetSnowBaseline", (void*)&srSetSnowBaseline);
  engine->RegisterScriptFunction("srSetSnowTransparency", (void*)&srSetSnowTransparency);
  engine->RegisterScriptFunction("srSetSnowDefaultView", (void*)&srSetSnowDefaultView);
  engine->RegisterScriptFunction("srSetSnowWindSpeed", (void*)&srSetSnowWindSpeed);
  engine->RegisterScriptFunction("srSetSnowAmount", (void*)&srSetSnowAmount);
  engine->RegisterScriptFunction("srSetSnowView", (void*)&srSetSnowView);

  engine->RegisterScriptFunction("srSetRainDriftRange", (void*)&srSetRainDriftRange);
  engine->RegisterScriptFunction("srSetRainDriftSpeed", (void*)&srSetRainDriftSpeed);
  engine->RegisterScriptFunction("srSetRainFallSpeed", (void*)&srSetRainFallSpeed);
  engine->RegisterScriptFunction("srChangeRainAmount", (void*)&srChangeRainAmount);
  engine->RegisterScriptFunction("srSetRainBaseline", (void*)&srSetRainBaseline);
  engine->RegisterScriptFunction("srSetRainTransparency", (void*)&srSetRainTransparency);
  engine->RegisterScriptFunction("srSetRainDefaultView", (void*)&srSetRainDefaultView);
  engine->RegisterScriptFunction("srSetRainWindSpeed", (void*)&srSetRainWindSpeed);
  engine->RegisterScriptFunction("srSetRainAmount", (void*)&srSetRainAmount);
  engine->RegisterScriptFunction("srSetRainView", (void*)&srSetRainView);  

  engine->RegisterScriptFunction("srSetWindSpeed", (void*)&srSetWindSpeed);
  engine->RegisterScriptFunction("srSetBaseline", (void*)&srSetBaseline);

  engine->RequestEventHook(AGSE_PREGUIDRAW);
  engine->RequestEventHook(AGSE_PRESCREENDRAW);
  
  rain = new Weather;
  snow = new Weather(true);
}

void AGS_EngineShutdown()
{
}

int AGS_EngineOnEvent(int event, int data)
{
  if (event == AGSE_PREGUIDRAW)
  {
    if (rain->IsActive())
      rain->Update();
    
    if (snow->IsActive())
      snow->UpdateWithDrift();
  }
  else if (event == AGSE_PRESCREENDRAW)
  {
    // Get screen size once here
    engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);
    engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
  }
  
  return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved)
{
  return 0;
}

void AGS_EngineInitGfx(const char *driverID, void *data)
{
}