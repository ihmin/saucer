#include "win32.icon.impl.hpp"

#include "win32.utils.hpp"
#include "win32.error.hpp"

#include <wrl.h>
#include <shlwapi.h>

namespace saucer
{
    using Microsoft::WRL::ComPtr;

    // https://stackoverflow.com/questions/5345803/does-gdi-have-standard-image-encoder-clsids
    static constexpr CLSID png_encoder = {0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};

    icon::icon() : m_impl(std::make_unique<impl>()) {}

    icon::icon(impl data) : m_impl(std::make_unique<impl>(std::move(data))) {}

    icon::icon(const icon &other) : icon(*other.m_impl) {}

    icon::icon(icon &&other) noexcept : icon()
    {
        swap(*this, other);
    }

    icon::~icon() = default;

    icon &icon::operator=(icon other) noexcept
    {
        swap(*this, other);
        return *this;
    }

    void swap(icon &first, icon &second) noexcept
    {
        using std::swap;
        swap(first.m_impl, second.m_impl);
    }

    bool icon::empty() const
    {
        return !m_impl->bitmap || m_impl->bitmap->GetLastStatus() != Gdiplus::Status::Ok;
    }

    stash<> icon::data() const
    {
        if (!m_impl->bitmap)
        {
            return stash<>::empty();
        }

        ComPtr<IStream> stream;

        if (!SUCCEEDED(CreateStreamOnHGlobal(nullptr, true, &stream)))
        {
            return stash<>::empty();
        }

        m_impl->bitmap->Save(stream.Get(), &png_encoder);

        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        stream->Seek(pos, STREAM_SEEK_SET, nullptr);

        return stash<>::from(utils::read(stream.Get()));
    }

    void icon::save(const fs::path &path) const
    {
        if (!m_impl->bitmap)
        {
            return;
        }

        m_impl->bitmap->Save(path.wstring().c_str(), &png_encoder);
    }

    result<icon> icon::from(const stash<> &ico)
    {
        ComPtr<IStream> data = SHCreateMemStream(ico.data(), static_cast<DWORD>(ico.size()));
        auto bitmap          = std::shared_ptr<Gdiplus::Bitmap>{Gdiplus::Bitmap::FromStream(data.Get())};

        if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Status::Ok)
        {
            return err(bitmap->GetLastStatus());
        }

        return icon{{std::move(bitmap)}};
    }

    result<icon> icon::from(const fs::path &file)
    {
        auto bitmap = std::shared_ptr<Gdiplus::Bitmap>{Gdiplus::Bitmap::FromFile(file.wstring().c_str())};

        if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Status::Ok)
        {
            return err(bitmap->GetLastStatus());
        }

        return icon{{std::move(bitmap)}};
    }
} // namespace saucer
