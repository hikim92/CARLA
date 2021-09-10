#include "DirectInputFFBBPLibrary.h"
#include "DirectInputFFBResources.h"

std::vector<float> UDirectInputFFBBPLibrary::Hist_position;
std::vector<float> UDirectInputFFBBPLibrary::Hist_goal;

UDirectInputFFBBPLibrary::UDirectInputFFBBPLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UDirectInputFFBBPLibrary::SafeReleaseDevice()
{
	if (g_pDevice) g_pDevice->Unacquire();

	SAFE_RELEASE(g_pEffect_spring);
	SAFE_RELEASE(g_pEffect_constforce);
	SAFE_RELEASE(g_pEffect_sin_wave);

	SAFE_RELEASE(g_pDevice);
	SAFE_RELEASE(g_pDI);
	UE_LOG(DirectInputFFBPluginLog, Log, TEXT("Free device!"));
}

bool UDirectInputFFBBPLibrary::InitDevice()
{
	UDirectInputFFBBPLibrary::SafeReleaseDevice();

	// INIT DIRECT INPUT =======================================
	HRESULT hr;
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("DirectInput8Create"));
		return false;
	}
	if (FAILED(hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, UDirectInputFFBBPLibrary::EnumFFDevicesCallback, nullptr,
		DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("EnumDevices"));
		return false;
	}
	if (!g_pDevice)
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("No g_pDevice found !"));
		return false;
	}
	if (FAILED(hr = g_pDevice->SetDataFormat(&c_dfDIJoystick2)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("SetDataFormat, c_dfDIJoystick2"));
		return false;
	}
	const auto hwnd = GetActiveWindow();
	if (!hwnd) {
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("GetActiveWindow() is null"));
		return false;
	}
	if (FAILED(hr = g_pDevice->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("SetCooperativeLevel, DISCL_EXCLUSIVE"));
		return false;
	}

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 0;
	if (FAILED(hr = g_pDevice->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("SetProperty, DIPROP_AUTOCENTER"));
		return false;
	}
	// =========================================================

	// INIT Spring effect ======================================
	DIEFFECT eff_spring = {};
	DICONDITION conditionEffect = { 0, 10000, 10000, 10000, 10000, 0 };
	eff_spring.dwSize = sizeof(DIEFFECT);
	eff_spring.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff_spring.dwDuration = INFINITE;
	eff_spring.dwSamplePeriod = 0;
	eff_spring.dwGain = DI_FFNOMINALMAX;
	eff_spring.dwTriggerButton = DIEB_NOTRIGGER;
	eff_spring.dwTriggerRepeatInterval = 0;
	eff_spring.cAxes = 1;
	eff_spring.rgdwAxes = rgdwAxesWheel;
	eff_spring.rglDirection = rglDirectionWheel;
	eff_spring.lpEnvelope = 0;
	eff_spring.cbTypeSpecificParams = sizeof(DICONDITION);
	eff_spring.lpvTypeSpecificParams = &conditionEffect;
	eff_spring.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->CreateEffect(GUID_Spring, &eff_spring, &g_pEffect_spring, nullptr)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("Init Spring effect"));
		return false;
	}
	// =========================================================

	// INIT ConstantForce effect ===============================
	DIEFFECT eff_const = {};
	DICONSTANTFORCE cf = { 0 };
	eff_const.dwSize = sizeof(DIEFFECT);
	eff_const.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff_const.dwDuration = INFINITE;
	eff_const.dwSamplePeriod = 0;
	eff_const.dwGain = DI_FFNOMINALMAX;
	eff_const.dwTriggerButton = DIEB_NOTRIGGER;
	eff_const.dwTriggerRepeatInterval = 0;
	eff_const.cAxes = 1;
	eff_const.rgdwAxes = rgdwAxesWheel;
	eff_const.rglDirection = rglDirectionWheel;
	eff_const.lpEnvelope = 0;
	eff_const.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
	eff_const.lpvTypeSpecificParams = &cf;
	eff_const.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->CreateEffect(GUID_ConstantForce, &eff_const, &g_pEffect_constforce, nullptr)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("Init ConstantForce effect"));
		return false;
	}
	// =========================================================

	// INIT SinWave effect =====================================
	DIEFFECT eff_sin = {};
	DIPERIODIC cPeriod = { 0, 0, 0, 0 };
	eff_sin.dwSize = sizeof(DIEFFECT);
	eff_sin.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff_sin.dwDuration = INFINITE;
	eff_sin.dwSamplePeriod = 0;
	eff_sin.dwGain = DI_FFNOMINALMAX;
	eff_sin.dwTriggerButton = DIEB_NOTRIGGER;
	eff_sin.dwTriggerRepeatInterval = 0;
	eff_sin.cAxes = 1;
	eff_sin.rgdwAxes = rgdwAxesWheel;
	eff_sin.rglDirection = rglDirectionWheel;
	eff_sin.lpEnvelope = 0;
	eff_sin.cbTypeSpecificParams = sizeof(DIPERIODIC);
	eff_sin.lpvTypeSpecificParams = &cPeriod;
	eff_sin.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->CreateEffect(GUID_Sine, &eff_sin, &g_pEffect_sin_wave, nullptr)))
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("Init SinWave effect"));
		return false;
	}
	// =========================================================

	if (!g_pEffect_spring && !g_pEffect_constforce && !g_pEffect_sin_wave)
	{
		UE_LOG(DirectInputFFBPluginLog, Error, TEXT("g_pEffect... is null"));
		return false;
	}
	return true;
}

float UDirectInputFFBBPLibrary::SteeringWheelPosition(bool inDegree)
{
	if (inDegree) return float(currentSteeringWheel * STREERINGWHEEL_ANGLE_MAX); // Driver assumes a range of 750°
	return currentSteeringWheel;
}

bool UDirectInputFFBBPLibrary::SetSpringParameters(int offset, int posCoef, int negCoef, int posSat, int negSat, int deadZone)
{
	if (!g_pDevice || !g_pEffect_spring) {
		return false;
	}

	HRESULT hr;
	LONG rglDirection[2] = { 0 , 0 };
	DICONDITION cd;
	cd.lOffset = long(UDirectInputFFBBPLibrary::clip(offset, -10000, 10000));
	cd.lPositiveCoefficient = long(UDirectInputFFBBPLibrary::clip(posCoef, -10000, 10000));
	cd.lNegativeCoefficient = long(UDirectInputFFBBPLibrary::clip(negCoef, -10000, 10000));
	cd.dwPositiveSaturation = unsigned long(UDirectInputFFBBPLibrary::clip(posSat, 0, 10000));
	cd.dwNegativeSaturation = unsigned long(UDirectInputFFBBPLibrary::clip(negSat, 0, 10000));
	cd.lDeadBand = long(UDirectInputFFBBPLibrary::clip(deadZone, 0, 10000));

	rglDirection[0] = long(UDirectInputFFBBPLibrary::clip(offset, -10000, 10000));

	DIEFFECT eff = {};
	eff.dwSize = sizeof(DIEFFECT);
	eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes = 1;
	eff.rglDirection = rglDirectionWheel;
	eff.lpEnvelope = 0;
	eff.cbTypeSpecificParams = sizeof(DICONDITION);
	eff.lpvTypeSpecificParams = &cd;
	eff.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->Acquire()))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("Acquire SetSpringParameters"));
		return false;
	}
	if (FAILED(hr = g_pEffect_spring->SetParameters(&eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START)))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SetSpringParameters"));
		g_pDevice->Unacquire();
		return false;
	}
	return true;
}


bool UDirectInputFFBBPLibrary::SetSinWaveParameters(int offset, int magnitude, int phase, int period)
{
	if (!g_pDevice || !g_pEffect_sin_wave) {
		return false;
	}

	HRESULT hr;
	LONG rglDirection[2] = { 0 , 0 };
	DIPERIODIC cd;
	cd.lOffset = long(UDirectInputFFBBPLibrary::clip(offset, -10000, 10000));
	cd.dwMagnitude = long(UDirectInputFFBBPLibrary::clip(magnitude, 0, 10000));
	cd.dwPhase = long(UDirectInputFFBBPLibrary::clip(phase, 0, 35999));
	cd.dwPeriod = unsigned long(period);

	DIEFFECT eff = {};
	eff.dwSize = sizeof(DIEFFECT);
	eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes = 1;
	eff.rglDirection = rglDirectionWheel;
	eff.lpEnvelope = 0;
	eff.cbTypeSpecificParams = sizeof(DIPERIODIC);
	eff.lpvTypeSpecificParams = &cd;
	eff.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->Acquire()))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("Acquire SetSinWaveParameters"));
		return false;
	}
	if (FAILED(hr = g_pEffect_sin_wave->SetParameters(&eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START)))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SetSinWaveParameters"));
		g_pDevice->Unacquire();
		return false;
	}
	return true;
}


bool UDirectInputFFBBPLibrary::SetConstantForceParameters(int force)
{
	if (!g_pDevice || !g_pEffect_constforce) {
		return false;
	}

	HRESULT hr;
	DICONSTANTFORCE diConstantForce;
	diConstantForce.lMagnitude = long(UDirectInputFFBBPLibrary::clip(force, -MAX_TORQUE, MAX_TORQUE));

	DIEFFECT eff = {};
	eff.dwSize = sizeof(DIEFFECT);
	eff.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
	eff.lpvTypeSpecificParams = &diConstantForce;
	eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes = 1;
	eff.lpEnvelope = 0;
	eff.dwStartDelay = 0;

	if (FAILED(hr = g_pDevice->Acquire()))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("Acquire g_pEffectConstforce"));
		return false;
	}
	if (FAILED(hr = g_pEffect_constforce->SetParameters(&eff, DIEP_TYPESPECIFICPARAMS | DIEP_START)))
	{
		UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("SetParameters g_pEffectConstforce"));
		g_pDevice->Unacquire();
		return false;
	}
	return true;
}

bool UDirectInputFFBBPLibrary::SetPIDConstantForceParameters(float consigne, bool& handsOnWheel, float kp, float ki, float kd, float deltaTime)
{
	if (!g_pDevice || !g_pEffect_constforce) {
		handsOnWheel = false;
		return false;
	}

	float goal = UDirectInputFFBBPLibrary::clip(consigne, -MAX_CONSIGNE, MAX_CONSIGNE);
	float position = UDirectInputFFBBPLibrary::SteeringWheelPosition(true);

	error = goal - position;
	differential = (error - last_error) / deltaTime;
	integral += (error + last_error) * deltaTime / 2;
	total_error += abs(error);

	UDirectInputFFBBPLibrary::Hist_position.push_back(position);
	UDirectInputFFBBPLibrary::Hist_goal.push_back(goal);

	torque = kp * error + ki * integral + kd * differential;
	UE_LOG(DirectInputFFBPluginLog, Warning, TEXT("LOG PID : %f,%f,%f,%d,%d,%d"), deltaTime, position, goal, torque, UDirectInputFFBBPLibrary::Hist_position.size(), UDirectInputFFBBPLibrary::Hist_goal.size());

	bool retValue = UDirectInputFFBBPLibrary::SetConstantForceParameters(torque);

	last_torque = torque;
	last_position = position;
	last_goal_theta = goal;
	last_error = error;

	// Hands On Wheel
	float best_diff = position - goal;
	for (int k = 1; k < SIZE_HIST_PID; k++) if (UDirectInputFFBBPLibrary::Hist_position.size() > k)
	{
		double diff = position - UDirectInputFFBBPLibrary::Hist_goal[UDirectInputFFBBPLibrary::Hist_position.size() - k];
		if (fabs(diff) < fabs(best_diff)) best_diff = diff;
	}

	if (fabs(best_diff) > max_diff) max_diff = fabs(best_diff);
	if (fabs(best_diff) > MAX_DIFFERENCE_HOW)
	{
		current_conseq_how++;
		last_how_suspicious = true;
	}
	else
	{
		if (last_how_suspicious && (current_conseq_how > max_conseq_how)) max_conseq_how = current_conseq_how;
		current_conseq_how = 0;
		last_how_suspicious = false;
	}

	if (currentBrake > 0.02f) {
		last_how_suspicious = true;
	}

	if (UDirectInputFFBBPLibrary::Hist_position.size() > SIZE_HIST_PID+1) {
		UDirectInputFFBBPLibrary::Hist_position.erase(UDirectInputFFBBPLibrary::Hist_position.begin());
	}
	if (UDirectInputFFBBPLibrary::Hist_goal.size() > SIZE_HIST_PID+1) {
		UDirectInputFFBBPLibrary::Hist_goal.erase(UDirectInputFFBBPLibrary::Hist_goal.begin());
	}

	handsOnWheel = last_how_suspicious;
	return retValue;
}

void UDirectInputFFBBPLibrary::CleanPIDValues()
{
	error = 0;
	integral = 0;
	differential = 0;
	total_error = 0;
	last_torque = 0;
	torque = 0;
	last_error = 0;
	last_position = 0;
	last_goal_theta = 0;
	current_conseq_how = 0;
	max_conseq_how = 0;
	max_diff = 0;
	last_how_suspicious = false;

	Hist_position.clear();
	Hist_goal.clear();
}

bool UDirectInputFFBBPLibrary::SetInitPositionStreeringWheel(float consigne)
{
	UDirectInputFFBBPLibrary::CleanPIDValues();
	int counterInf = 0;
	bool run = true;
	while (run)
	{
		UDirectInputFFBBPLibrary::SetPIDConstantForceParameters(consigne, run, -45.f, 0.f, -2.8f);
		counterInf++;
		run = counterInf < 100;
		FPlatformProcess::Sleep(0.02);
	}
	FPlatformProcess::Sleep(0.2);
	UDirectInputFFBBPLibrary::CleanPIDValues();
	return true;
}
