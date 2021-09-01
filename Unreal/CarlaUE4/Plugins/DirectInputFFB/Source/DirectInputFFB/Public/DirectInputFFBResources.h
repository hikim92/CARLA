#pragma once

#include <algorithm>

// DEFINE Input FANATEC
static const FKey WheelAxis("Wheel");
static const FKey AccelerationAxis("Accelerator");
static const FKey BrakeAxis("Brake");
static const FKey ClutchAxis("Clutch");

static const FKey Button01("B_01");
static const FKey Button02("B_02");
static const FKey Button03("B_03");
static const FKey Button04("B_04");
static const FKey Button05("B_05");
static const FKey Button06("B_06");

static const FKey ButtonForward("B_Forward");
static const FKey ButtonBackward("B_Backward");

#include "AllowWindowsPlatformTypes.h"

#define DIRECTINPUT_VERSION 0x0800
#define IDD_FORCE_FEEDBACK  102
#define MAX_TORQUE 800
#define MAX_CONSIGNE 25.0f
#define MAX_DIFFERENCE_HOW 10  // degrees difference MAX
#define STREERINGWHEEL_ANGLE_MAX 750.0f
#define SIZE_HIST_PID 10

#if !defined(M_PI)
#define M_PI 3.14159265358979323846f
#endif  

#include <windows.h>
#include <winnt.h>
#include <CommCtrl.h>
#include <dinput.h>
#include <ctime>
#include <Timeapi.h>

LPDIRECTINPUT8          g_pDI = nullptr;
LPDIRECTINPUTDEVICE8    g_pDevice = nullptr;

LPDIRECTINPUTEFFECT     g_pEffect_spring = nullptr;
LPDIRECTINPUTEFFECT     g_pEffect_constforce = nullptr;
LPDIRECTINPUTEFFECT     g_pEffect_sin_wave = nullptr;

LONG rglDirectionWheel[2] = { 0, 0 };
DWORD rgdwAxesWheel[2] = { DIJOFS_X };

// CURRENT INPUT VALUES =========================
float currentSteeringWheel = 0.f;
float currentAccelerator = 0.f;
float currentBrake = 0.f;
float currentClutch = 0.f;

bool currentB_01 = false;
bool currentB_02 = false;
bool currentB_03 = false;
bool currentB_04 = false;
bool currentB_05 = false;
bool currentB_06 = false;

bool currentB_Forward = false;
bool currentB_Backward = false;

// PID ==========================================
float error = 0;
float integral = 0;
float differential = 0;
float total_error = 0;
int last_torque = 0;
int torque = 0;
float last_error = 0;
float last_position = 0;
float last_goal_theta = 0;
// Hands On Wheel
int current_conseq_how = 0;
int max_conseq_how = 0;
float max_diff = 0;
bool last_how_suspicious = false;
// ============================================

#include "HideWindowsPlatformTypes.h"
