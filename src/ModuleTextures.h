#pragma once

#include "Module.h"
#include <vector>

typedef struct IDirect3DTexture9 *LPDIRECT3DTEXTURE9, *PDIRECT3DTEXTURE9;

class ModuleTextures : public Module
{
public:

	// Module virtual functions

	bool init() override;

	bool cleanUp() override;

	bool invalidateDeviceObjects() override;

	bool restoreDeviceObjects() override;


	// ModuleTextures methods

	LPDIRECT3DTEXTURE9 loadTexture(const char *filename);

	void freeTexture(LPDIRECT3DTEXTURE9 texture);


	// Textures
	LPDIRECT3DTEXTURE9 banner = nullptr;


private:

	std::vector<LPDIRECT3DTEXTURE9> _textures;
};
