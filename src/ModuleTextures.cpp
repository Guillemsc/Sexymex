#pragma once

#include "ModuleTextures.h"
#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800

extern LPDIRECT3DDEVICE9 g_pd3dDevice; // CHAP: Direct3d9 device pointer, defined in ModuleWindow.cpp


bool ModuleTextures::init()
{
	banner = loadTexture("assets/banner.png");

	return true;
}

bool ModuleTextures::cleanUp()
{
	for (auto texture : _textures)
	{
		texture->Release();
	}
	_textures.clear();

	return true;
}

bool ModuleTextures::invalidateDeviceObjects()
{
	return cleanUp();
}

bool ModuleTextures::restoreDeviceObjects()
{
	return init();
}

LPDIRECT3DTEXTURE9 ModuleTextures::loadTexture(const char *filename)
{
	// Read image pixels
	int width, height, nchannels;
	unsigned char *pixels = stbi_load( (char *)filename, &width, &height, &nchannels, 0);
	if (pixels == nullptr) {
		wLog << "ModuleTextures::loadTexture() - stbi_load() failed.";
		return nullptr;
	}
	int bytes_per_pixel = nchannels;

	// Create texture in graphics system
	LPDIRECT3DTEXTURE9 texture = nullptr;
	if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL) < 0) {
		wLog << "ModuleTextures::loadTexture() - g_pd3dDevice->CreateTexture() failed.";
		return nullptr;
	}

	// Upload texture data to graphics system
	D3DLOCKED_RECT tex_locked_rect;
	if (texture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK) {
		wLog << "ModuleTextures::loadTexture() - texture->LockRect() failed.";
		return nullptr;
	}
	for (int y = 0; y < height; y++)
		memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
	texture->UnlockRect(0);

	// Free image pixels
	stbi_image_free(pixels);

	// Add the texture to the array of textures
	unsigned int i;
	for (i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i] == nullptr)
		{
			_textures[i] = texture;
			break;
		}
	}
	if (i == _textures.size()) {
		_textures.push_back(texture);
	}

	// Return the texture
	return texture;
}

void ModuleTextures::freeTexture(LPDIRECT3DTEXTURE9 texture)
{
	if (texture != nullptr)
	{
		texture->Release();

		std::vector<LPDIRECT3DTEXTURE9> remainingTextures;
		unsigned int i;
		for (i = 0; i < _textures.size(); ++i)
		{
			if (_textures[i] == texture)
			{
				_textures[i] = nullptr;
				break;
			}
		}
	}
}
