#pragma once

#include "core/base.h"
#include "core/buffer.h"

namespace penumbra
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RG8,
		RGBA8,
		RGBA16F,
		RGB32F,
		RGBA32F,
	};

	struct TextureSpecification_t
	{
		uint32_t m_nWidth = 1;
		uint32_t m_nHeight = 1;
		ImageFormat m_Format = ImageFormat::RGBA8;
		bool m_bGenerateMips = true;

		bool operator==(const TextureSpecification_t& other) const
		{
			return m_nWidth == other.m_nWidth &&
				m_nHeight == other.m_nHeight &&
				m_Format == other.m_Format &&
				m_bGenerateMips == other.m_bGenerateMips;
		}
	};

	class CTexture
	{
	public:
		virtual ~CTexture() = default;

		virtual const TextureSpecification_t& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual intptr_t GetRendererID() const = 0;

		virtual void SetData(Buffer_t data) = 0;

		virtual void Bind(uint32_t nSlot = 0) const = 0;

		virtual const bool IsLoaded() const = 0;

		static uint32_t GetBPP(ImageFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

		virtual bool operator==(const CTexture& other) const = 0;
	};

	class CTexture2D : public CTexture
	{
	public:
		CTexture2D(const TextureSpecification_t& specification, Buffer_t data = Buffer_t());
		~CTexture2D() = default;

		const TextureSpecification_t& GetSpecification() const override { return m_Specification; }

		uint32_t GetWidth() const override { return m_Specification.m_nWidth; }
		uint32_t GetHeight() const override { return m_Specification.m_nHeight; }
		intptr_t GetRendererID() const override { return reinterpret_cast<intptr_t>(m_spShaderResourceView.Get()); }

		void SetData(Buffer_t data) override;

		void Bind(uint32_t nSlot = 0) const override;

		const bool IsLoaded() const override { return m_bIsLoaded; }

		bool operator==(const CTexture& other) const override
		{
			return reinterpret_cast<intptr_t>(m_spShaderResourceView.Get()) == other.GetRendererID();
		};
	private:
		TextureSpecification_t m_Specification;

		ComPtr<ID3D11ShaderResourceView> m_spShaderResourceView;
		ComPtr<ID3D11Texture2D> m_spTexture;
		std::string m_Path;
		bool m_bIsLoaded = false;

		ComPtr<ID3D11DeviceContext> m_spContext;
	};
}