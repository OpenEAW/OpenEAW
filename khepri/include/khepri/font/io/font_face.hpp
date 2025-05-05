#pragma once

#include <khepri/font/font_face_desc.hpp>
#include <khepri/io/stream.hpp>

namespace khepri::font::io {

FontFaceDesc load_font_face(khepri::io::Stream& stream);

} // namespace khepri::font::io
