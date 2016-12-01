#pragma once

#define DEBUG_LOG

class Chip8Emulator
{
public:
	Chip8Emulator();
	~Chip8Emulator();
	void Init();
	void EmulateCycle();
	void LoadRom(std::wstring fileName);
	bool ShouldDraw();
	void KeyPress(int keyPressed);
	void KeyRelease(int keyReleased);
	unsigned char gfx[2048];

private:
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char V[16];
	unsigned short index;
	unsigned short programCounter;	
	unsigned char delayTimer;
	unsigned char soundTimer;
	unsigned short stack[16];
	unsigned short stackPointer;
	unsigned char key[16];
	bool requiresDraw;

	// Opcode function table
	static void (Chip8Emulator::*OpcodeTable[16])();
	static void (Chip8Emulator::*Opcode8Table[16])();

	void NotImplemented();
	void Opcode00();
	void Opcode80();
	void OpcodeEX();
	void OpcodeFX();
	void ClearScreen();
	void SetRegister();
	void SkipIfNotEqual();
	void SetIndex();
	void StoreRegisters();
	void Jump();
	void CallSub();
	void Return();
	void SkipIfEqual();
	void AddRegisterToIndex();
	void FillRegisters();
	void GetFontSetSprite();
	void DrawSprite();
	void AddToRegister();
	void RegisterOr();
	void RegisterAnd();
	void RegisterXor();
	void RegisterCopy();
	void RegisterShiftRight();
	void RegisterShiftLeft();
	void SkipIfKeyPressed();
	void SkipIfKeyNotPressed();
	void SkipIfRegistersNotEqual();
	void AddRegisters();
	void SubtractRegisters();
	void RandomNumber();
	void GetDelayTimer();
	void SetSoundTimer();
};

