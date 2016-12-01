#include "stdafx.h"
#include "Chip8Emulator.h"

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void (Chip8Emulator::*(Chip8Emulator::OpcodeTable)[])() =
{
	&Opcode00, &Jump, &CallSub, &SkipIfEqual, &SkipIfNotEqual, &NotImplemented, &SetRegister, &AddToRegister,
	&Opcode80, &SkipIfRegistersNotEqual, &SetIndex, &NotImplemented, &RandomNumber, &DrawSprite, &OpcodeEX, &OpcodeFX
};

void (Chip8Emulator::*(Chip8Emulator::Opcode8Table)[])() =
{
	&RegisterCopy, &RegisterOr, &RegisterAnd, &RegisterXor, &AddRegisters, &SubtractRegisters, &RegisterShiftRight, &NotImplemented,
	&NotImplemented, &NotImplemented, &NotImplemented, &NotImplemented, &NotImplemented, &NotImplemented, &RegisterShiftLeft, &NotImplemented
};

Chip8Emulator::Chip8Emulator()
{
}


Chip8Emulator::~Chip8Emulator()
{
}

void Chip8Emulator::Init()
{
	opcode = 0;
	memset(memory, 0, sizeof(memory));
	memset(V, 0, sizeof(V));
	index = 0;
	programCounter = 0x200;
	delayTimer = 0;
	soundTimer = 0;
	memset(stack, 0, sizeof(stack));
	stackPointer = 0;
	memset(key, 0, sizeof(key));

	ClearScreen();

	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}
}

void Chip8Emulator::EmulateCycle()
{
	requiresDraw = false;
	opcode = memory[programCounter] << 8 | memory[programCounter + 1];

	if (delayTimer != 0)
	{
		delayTimer--;
	}
	if (soundTimer != 0)
	{
		soundTimer--;
		if (soundTimer == 0)
		{
			Beep(750, 100);
		}
	}

	(this->*OpcodeTable[(opcode & 0xF000) >> 12])();
	
	programCounter = programCounter + 2;
}

void Chip8Emulator::LoadRom(std::wstring fileName)
{
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		unsigned int location = 0x200;
		while (!file.eof())
		{
			file.read((char*)&memory[location], 1);
			location++;
		}
	}


}

bool Chip8Emulator::ShouldDraw()
{
	return requiresDraw;
}

void Chip8Emulator::KeyPress(int keyPressed)
{
	key[keyPressed] = 1;
}

void Chip8Emulator::KeyRelease(int keyReleased)
{
	key[keyReleased] = 0;
}

void Chip8Emulator::NotImplemented()
{
	printf("%4x: %4x", programCounter, opcode);
	getchar();
}

void Chip8Emulator::Opcode00()
{
	switch (opcode)
	{
	case 0x00E0:
		ClearScreen();
		break;
	case 0x00EE:
		Return();
		break;
	default:
		NotImplemented();
		break;
	}
}

void Chip8Emulator::Opcode80()
{
	(this->*Opcode8Table[(opcode & 0x000F)])();
}

void Chip8Emulator::OpcodeEX()
{
	switch (opcode & 0x00FF)
	{
	case 0x009E:
		SkipIfKeyPressed();
		break;
	case 0x00A1:
		SkipIfKeyNotPressed();
		break;
	}
}

void Chip8Emulator::OpcodeFX()
{
	switch (opcode & 0x00FF)
	{
	case 0x0007:
		GetDelayTimer();
		break;
	case 0x0018:
		SetSoundTimer();
		break;
	case 0x001E:
		AddRegisterToIndex();
		break;
	case 0x0029:
		GetFontSetSprite();
		break;
	case 0x0055:
		StoreRegisters();
		break;
	case 0x0065:
		FillRegisters();
		break;
	default:
		NotImplemented();
		break;
	}
}

void Chip8Emulator::ClearScreen()
{
	memset(gfx, 0, sizeof(gfx));
	requiresDraw = true;

#ifdef DEBUG_LOG
	printf("%4x: %4x cls\r\n", programCounter, opcode);
#endif
}

void Chip8Emulator::SetRegister()
{
	V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] -> %x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::SkipIfNotEqual()
{
#ifdef DEBUG_LOG
	unsigned short oldPC = programCounter;
#endif
	if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
	{
		programCounter = programCounter + 2;
	}

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x]:%x != %x : %s\r\n", oldPC, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], (opcode & 0x00FF), V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ? "true" : "false");
#endif
}

void Chip8Emulator::SetIndex()
{
	index = opcode & 0x0FFF;

#ifdef DEBUG_LOG
	printf("%4x: %4x i -> %x\r\n", programCounter, opcode, index);
#endif
}

void Chip8Emulator::StoreRegisters()
{
	int numRegisters = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= numRegisters; i++)
	{
		memory[index + i] = V[i];
	}

#ifdef DEBUG_LOG
	printf("%4x: %4x %d registers to memory @ %x\r\n", programCounter, opcode, numRegisters, index);
#endif
}

void Chip8Emulator::Jump()
{
#ifdef DEBUG_LOG
	unsigned short oldPC = programCounter;
#endif
	programCounter = (opcode & 0x0FFF) - 2;

#ifdef DEBUG_LOG
	printf("%4x: %4x jump %x\r\n", oldPC, opcode, programCounter + 2);
#endif
}

void Chip8Emulator::CallSub()
{
	stack[stackPointer] = programCounter;
	stackPointer++;
	// Subtract two to avoid special casing the +2 in EmulateCycle 
	programCounter = (opcode & 0x0FFF) - 2;

#ifdef DEBUG_LOG
	printf("%4x: %4x call sub %x\r\n", stack[stackPointer - 1], opcode, programCounter + 2);
#endif
}

void Chip8Emulator::Return()
{
#ifdef DEBUG_LOG
	unsigned short oldPC = programCounter;
#endif
	stackPointer--;
	programCounter = stack[stackPointer];

#ifdef DEBUG_LOG
	printf("%4x: %4x return to %x\r\n", oldPC, opcode, programCounter + 2);
#endif
}

void Chip8Emulator::SkipIfEqual()
{
#ifdef DEBUG_LOG
	unsigned short oldPC = programCounter;
#endif
	if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
	{
		programCounter = programCounter + 2;
	}

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x]:%x == %x : %s\r\n", oldPC, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], (opcode & 0x00FF), V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) ? "true" : "false");
#endif
}

void Chip8Emulator::AddRegisterToIndex()
{
	index = index + V[(opcode & 0x0F00) >> 8];

#ifdef DEBUG_LOG
	printf("%4x: %4x index + V[%x] = %x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, index);
#endif
}

void Chip8Emulator::FillRegisters()
{
	int numRegisters = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= numRegisters; i++)
	{
		V[i] = memory[index + i];
	}

#ifdef DEBUG_LOG
	printf("%4x: %4x %d registers from memory @ %x\r\n", programCounter, opcode, numRegisters, index);
#endif
}

void Chip8Emulator::GetFontSetSprite()
{
	index = V[(opcode & 0x0F00) >> 8] * 5;

#ifdef DEBUG_LOG
	printf("%4x: %4x get fontset sprite for %x (%x)\r\n", programCounter, opcode, V[(opcode & 0x0F00) >> 8], index);
#endif
}

void Chip8Emulator::DrawSprite()
{
	unsigned short x = V[(opcode & 0x0F00) >> 8];
	unsigned short y = V[(opcode & 0x00F0) >> 4];
	unsigned short height = opcode & 0x000F;
	unsigned short pixel;

	V[0xF] = 0;
	for (int yline = 0; yline < height; yline++)
	{
		pixel = memory[index + yline];
		for (int xline = 0; xline < 8; xline++)
		{
			if ((pixel & (0x80 >> xline)) != 0)
			{
				if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					V[0xF] = 1;
				gfx[x + xline + ((y + yline) * 64)] ^= 1;
			}
		}
	}
	requiresDraw = true;

#ifdef DEBUG_LOG
	printf("%4x: %4x draw sprite\r\n", programCounter, opcode);
#endif
}

void Chip8Emulator::AddToRegister()
{
	V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);

#ifdef DEBUG_LOG
	printf("%4x: %4x Add %4x to V[%x] -> %4x\r\n", programCounter, opcode, (opcode & 0x00FF), (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::RegisterOr()
{
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] | V[%x] -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::RegisterAnd()
{
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] & V[%x] -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::RegisterXor()
{
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] ^ V[%x] -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::RegisterCopy()
{
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::RegisterShiftRight()
{
	V[0xF] = V[(opcode & 0x0F00) >> 8] & 1;
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] shift right %4x, V[f]=%x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], V[0xf]);
#endif
}

void Chip8Emulator::RegisterShiftLeft()
{
	V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] shift left %4x, V[f]=%x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], V[0xf]);
#endif
}

void Chip8Emulator::SkipIfKeyPressed()
{
	if (key[V[(opcode & 0x0F00) >> 8]] == 1)
	{
		programCounter += 2;
	}
#ifdef DEBUG_LOG
	printf("%4x: %4x Is key pressed %x: %x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, key[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::SkipIfKeyNotPressed()
{
	if (key[V[(opcode & 0x0F00) >> 8]] == 0)
	{
		programCounter += 2;
	}
#ifdef DEBUG_LOG
	printf("%4x: %4x Is key not pressed %x: %x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, key[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::SkipIfRegistersNotEqual()
{
#ifdef DEBUG_LOG
	unsigned short oldPC = programCounter;
#endif
	if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
	{
		programCounter = programCounter + 2;
	}

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x]:%x != V[%x]:%x : %s\r\n", oldPC, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], (opcode & 0x00F0) >> 4, V[(opcode & 0x00F0) >> 4], V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ? "true" : "false");
#endif
}

void Chip8Emulator::AddRegisters()
{
	if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
	{
		V[0xF] = 1;
	}
	else
	{
		V[0xF] = 0;
	}
	V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x Add V[%x] to V[%x] -> %4x, V[F]=%x\r\n", programCounter, opcode, (opcode & 0x00F0) >> 4, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], V[0xf]);
#endif
}

void Chip8Emulator::SubtractRegisters()
{
	if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
	{
		V[0xF] = 1;
	}
	else
	{
		V[0xF] = 0;
	}
	V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];

#ifdef DEBUG_LOG
	printf("%4x: %4x Subtract V[%x] from V[%x] -> %4x, V[F]=%x\r\n", programCounter, opcode, (opcode & 0x00F0) >> 4, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8], V[0xf]);
#endif
}

void Chip8Emulator::RandomNumber()
{
	V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & rand();

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] random number -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::GetDelayTimer()
{
	V[(opcode & 0x0F00) >> 8] = delayTimer;

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] get delay timer -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}

void Chip8Emulator::SetSoundTimer()
{
	soundTimer = V[(opcode & 0x0F00) >> 8];

#ifdef DEBUG_LOG
	printf("%4x: %4x V[%x] set delay timer -> %4x\r\n", programCounter, opcode, (opcode & 0x0F00) >> 8, V[(opcode & 0x0F00) >> 8]);
#endif
}
