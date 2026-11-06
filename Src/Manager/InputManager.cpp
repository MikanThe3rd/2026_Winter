#include <DxLib.h>
#include "InputManager.h"

InputManager* InputManager::instance_ = nullptr;

void InputManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new InputManager();
	}
	instance_->Init();
}

InputManager& InputManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		InputManager::CreateInstance();
	}
	return *instance_;
}

void InputManager::Init(void)
{

	// ゲームで使用したいキーを、
	// 事前にここで登録しておいてください
	InputManager::GetInstance().Add(KEY_INPUT_SPACE);
	InputManager::GetInstance().Add(KEY_INPUT_N);
	InputManager::GetInstance().Add(KEY_INPUT_Z);

	InputManager::GetInstance().Add(KEY_INPUT_LEFT);
	InputManager::GetInstance().Add(KEY_INPUT_RIGHT);
	InputManager::GetInstance().Add(KEY_INPUT_UP);
	InputManager::GetInstance().Add(KEY_INPUT_DOWN);

	InputManager::GetInstance().Add(KEY_INPUT_W);
	InputManager::GetInstance().Add(KEY_INPUT_A);
	InputManager::GetInstance().Add(KEY_INPUT_S);
	InputManager::GetInstance().Add(KEY_INPUT_D);

	InputManager::GetInstance().Add(KEY_INPUT_M);
	InputManager::GetInstance().Add(KEY_INPUT_G);
	InputManager::GetInstance().Add(KEY_INPUT_R);
	InputManager::GetInstance().Add(KEY_INPUT_B);
	InputManager::GetInstance().Add(KEY_INPUT_K);

	InputManager::GetInstance().Add(KEY_INPUT_LSHIFT);

	InputManager::GetInstance().Add(KEY_INPUT_RETURN);

	InputManager::GetInstance().Add(KEY_INPUT_LALT);

	InputManager::GetInstance().Add(KEY_INPUT_ESCAPE);

	InputManager::MouseInfo info;

	// 左クリック
	info = InputManager::MouseInfo();
	info.key = MOUSE_INPUT_LEFT;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	mouseInfos_.emplace(info.key, info);

	// 右クリック
	info = InputManager::MouseInfo();
	info.key = MOUSE_INPUT_RIGHT;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	mouseInfos_.emplace(info.key, info);
}

void InputManager::Update(void)
{

	// キーボード検知
	for (auto& p : keyInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = CheckHitKey(p.second.key);
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// マウス検知
	mouseInput_ = GetMouseInput();
	GetMousePoint(&mousePos_.x, &mousePos_.y);

	for (auto& p : mouseInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = mouseInput_ == p.second.key;
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// パッド情報
	SetJPadInState(JOYPAD_NO::KEY_PAD1);
	SetJPadInState(JOYPAD_NO::PAD1);
	SetJPadInState(JOYPAD_NO::PAD2);
	SetJPadInState(JOYPAD_NO::PAD3);
	SetJPadInState(JOYPAD_NO::PAD4);

}

void InputManager::Destroy(void)
{
	keyInfos_.clear();
	mouseInfos_.clear();
	delete instance_;
}

void InputManager::Add(int key)
{
	InputManager::Info info = InputManager::Info();
	info.key = key;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	keyInfos_.emplace(key, info);
}

void InputManager::Clear(void)
{
	keyInfos_.clear();
}

bool InputManager::IsNew(int key) const
{
	return Find(key).keyNew;
}

bool InputManager::IsTrgDown(int key) const
{
	return Find(key).keyTrgDown;
}

bool InputManager::IsTrgUp(int key) const
{
	return Find(key).keyTrgUp;
}

Vector2 InputManager::GetMousePos(void) const
{
	return mousePos_;
}

int InputManager::GetMouse(void) const
{
	return mouseInput_;
}

bool InputManager::IsClickMouseLeft(void) const
{
	return mouseInput_ == MOUSE_INPUT_LEFT;
}

bool InputManager::IsClickMouseRight(void) const
{
	return mouseInput_ == MOUSE_INPUT_RIGHT;
}

bool InputManager::IsTrgMouseLeft(void) const
{
	return FindMouse(MOUSE_INPUT_LEFT).keyTrgDown;
}

bool InputManager::IsTrgMouseRight(void) const
{
	return FindMouse(MOUSE_INPUT_RIGHT).keyTrgDown;
}

InputManager::InputManager(void)
{
	mouseInput_ = -1;
	for (auto& p : padInfos_)
	{
		p = JOYPAD_IN_STATE();
	}
}

const InputManager::Info& InputManager::Find(int key) const
{

	auto it = keyInfos_.find(key);
	if (it != keyInfos_.end())
	{
		return it->second;
	}

	return infoEmpty_;

}

const InputManager::MouseInfo& InputManager::FindMouse(int key) const
{
	auto it = mouseInfos_.find(key);
	if (it != mouseInfos_.end())
	{
		return it->second;
	}

	return mouseInfoEmpty_;
}

InputManager::JOYPAD_TYPE InputManager::GetJPadType(JOYPAD_NO no)
{
	return static_cast<InputManager::JOYPAD_TYPE>(GetJoypadType(static_cast<int>(no)));
}

DINPUT_JOYSTATE InputManager::GetJPadDInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadDirectInputState(static_cast<int>(no), &joyDInState_);
	return joyDInState_;
}

XINPUT_STATE InputManager::GetJPadXInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadXInputState(static_cast<int>(no), &joyXInState_);
	return joyXInState_;
}

void InputManager::SetJPadInState(JOYPAD_NO jpNo)
{

	int no = static_cast<int>(jpNo);
	auto stateNew = GetJPadInputState(jpNo);
	auto& stateNow = padInfos_[no];

	int max = static_cast<int>(JOYPAD_BTN::MAX);
	for (int i = 0; i < max; i++)
	{

		stateNow.ButtonsOld[i] = stateNow.ButtonsNew[i];
		stateNow.ButtonsNew[i] = stateNew.ButtonsNew[i];

		stateNow.IsOld[i] = stateNow.IsNew[i];
		//stateNow.IsNew[i] = stateNow.ButtonsNew[i] == 128 || stateNow.ButtonsNew[i] == 255;
		stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 0;

		stateNow.IsTrgDown[i] = stateNow.IsNew[i] && !stateNow.IsOld[i];
		stateNow.IsTrgUp[i] = !stateNow.IsNew[i] && stateNow.IsOld[i];


		stateNow.AKeyLX = stateNew.AKeyLX;
		stateNow.AKeyLY = stateNew.AKeyLY;
		stateNow.AKeyRX = stateNew.AKeyRX;
		stateNow.AKeyRY = stateNew.AKeyRY;

	}

	// 方向入力(十字キー、または左スティックを一定以上倒した状態)
	const int th = static_cast<int>(STICK_MAX * STICK_DIR_RATE);
	const int up = static_cast<int>(JOYPAD_DIR::UP);
	const int down = static_cast<int>(JOYPAD_DIR::DOWN);
	const int left = static_cast<int>(JOYPAD_DIR::LEFT);
	const int right = static_cast<int>(JOYPAD_DIR::RIGHT);

	bool dirNow[static_cast<int>(JOYPAD_DIR::MAX)];
	dirNow[up] = stateNew.DPad[up] || stateNew.AKeyLY < -th;      // DInput は上が負
	dirNow[down] = stateNew.DPad[down] || stateNew.AKeyLY > th;
	dirNow[left] = stateNew.DPad[left] || stateNew.AKeyLX < -th;
	dirNow[right] = stateNew.DPad[right] || stateNew.AKeyLX > th;

	for (int i = 0; i < static_cast<int>(JOYPAD_DIR::MAX); i++)
	{
		stateNow.DPad[i] = stateNew.DPad[i];
		stateNow.DirOld[i] = stateNow.DirNew[i];
		stateNow.DirNew[i] = dirNow[i];
	}

}

InputManager::JOYPAD_IN_STATE InputManager::GetJPadInputState(JOYPAD_NO no)
{

	JOYPAD_IN_STATE ret = JOYPAD_IN_STATE();

	auto type = GetJPadType(no);
	
	switch (type)
	{
	case InputManager::JOYPAD_TYPE::OTHER:
		break;
	case InputManager::JOYPAD_TYPE::XBOX_360:
	{
	}
		break;
	case InputManager::JOYPAD_TYPE::XBOX_ONE:
	{

		auto d = GetJPadDInputState(no);
		auto x = GetJPadXInputState(no);

		int idx;

		//   Y
		// X   B
		//   A

		idx = static_cast<int>(JOYPAD_BTN::TOP);
		ret.ButtonsNew[idx] = d.Buttons[3];// Y

		idx = static_cast<int>(JOYPAD_BTN::LEFT);
		ret.ButtonsNew[idx] = d.Buttons[2];// X

		idx = static_cast<int>(JOYPAD_BTN::RIGHT);
		ret.ButtonsNew[idx] = d.Buttons[1];// B

		idx = static_cast<int>(JOYPAD_BTN::DOWN);
		ret.ButtonsNew[idx] = d.Buttons[0];// A

		idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
		ret.ButtonsNew[idx] = x.RightTrigger;// R_TRIGGER

		idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
		ret.ButtonsNew[idx] = x.LeftTrigger; // L_TRIGGER

		idx = static_cast<int>(JOYPAD_BTN::MENU);
		ret.ButtonsNew[idx] = d.Buttons[7];

		// 左スティック
		ret.AKeyLX = d.X;
		ret.AKeyLY = d.Y;
		
		// 右スティック
		ret.AKeyRX = d.Rx;
		ret.AKeyRY = d.Ry;

		// 十字キー
		ret.DPad[static_cast<int>(JOYPAD_DIR::UP)] = x.Buttons[XINPUT_BUTTON_DPAD_UP] != 0;
		ret.DPad[static_cast<int>(JOYPAD_DIR::DOWN)] = x.Buttons[XINPUT_BUTTON_DPAD_DOWN] != 0;
		ret.DPad[static_cast<int>(JOYPAD_DIR::LEFT)] = x.Buttons[XINPUT_BUTTON_DPAD_LEFT] != 0;
		ret.DPad[static_cast<int>(JOYPAD_DIR::RIGHT)] = x.Buttons[XINPUT_BUTTON_DPAD_RIGHT] != 0;

	}
		break;
	case InputManager::JOYPAD_TYPE::DUAL_SHOCK_4:
		break;
	case InputManager::JOYPAD_TYPE::DUAL_SENSE:
	{
		
		auto d = GetJPadDInputState(no);
		int idx;

		//   △
		// □  〇
		//   ×

		idx = static_cast<int>(JOYPAD_BTN::TOP);
		ret.ButtonsNew[idx] = d.Buttons[3];// △

		idx = static_cast<int>(JOYPAD_BTN::LEFT);
		ret.ButtonsNew[idx] = d.Buttons[0];// □

		idx = static_cast<int>(JOYPAD_BTN::RIGHT);
		ret.ButtonsNew[idx] = d.Buttons[2];// 〇

		idx = static_cast<int>(JOYPAD_BTN::DOWN);
		ret.ButtonsNew[idx] = d.Buttons[1];// ×

		// 左スティック
		ret.AKeyLX = d.X;
		ret.AKeyLY = d.Y;
		
		// 右スティック
		ret.AKeyRX = d.Z;
		ret.AKeyRY = d.Rz;

		// 十字キー(ハットスイッチ: 0=上, 9000=右, 18000=下, 27000=左。斜めは間の値)
		const unsigned int pov = d.POV[0];
		if (pov <= 36000)
		{
			ret.DPad[static_cast<int>(JOYPAD_DIR::UP)] = (pov <= 4500 || pov >= 31500);
			ret.DPad[static_cast<int>(JOYPAD_DIR::RIGHT)] = (pov >= 4500 && pov <= 13500);
			ret.DPad[static_cast<int>(JOYPAD_DIR::DOWN)] = (pov >= 13500 && pov <= 22500);
			ret.DPad[static_cast<int>(JOYPAD_DIR::LEFT)] = (pov >= 22500 && pov <= 31500);
		}

	}
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_JOY_CON_L:
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_JOY_CON_R:
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_PRO_CTRL:
		break;
	case InputManager::JOYPAD_TYPE::MAX:
		break;
	}

	return ret;

}

bool InputManager::IsPadBtnNew(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsNew[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgDown(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsTrgDown[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgUp(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsTrgUp[static_cast<int>(btn)];
}

bool InputManager::IsPadDirNew(JOYPAD_NO no, JOYPAD_DIR dir) const
{
	return padInfos_[static_cast<int>(no)].DirNew[static_cast<int>(dir)];
}

bool InputManager::IsPadDirTrgDown(JOYPAD_NO no, JOYPAD_DIR dir) const
{
	const auto& pad = padInfos_[static_cast<int>(no)];
	const int i = static_cast<int>(dir);
	return pad.DirNew[i] && !pad.DirOld[i];
}

Vector2 InputManager::GetPadStickL(JOYPAD_NO no) const
{
	const auto& pad = padInfos_[static_cast<int>(no)];
	Vector2 ret;
	ret.x = pad.AKeyLX;
	ret.y = pad.AKeyLY;
	return ret;
}

Vector2 InputManager::GetPadStickR(JOYPAD_NO no) const
{
	const auto& pad = padInfos_[static_cast<int>(no)];
	Vector2 ret;
	ret.x = pad.AKeyRX;
	ret.y = pad.AKeyRY;
	return ret;
}


