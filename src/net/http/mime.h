#ifndef LILY_SRC_NET_HTTP_MIME_H_
#define LILY_SRC_NET_HTTP_MIME_H_

#include <unordered_map>
#include <string>
#include <algorithm>

inline static std::unordered_map<std::string, std::string> mime_map = {
    {".html", "text/html"},
    {".xhtml", "text/html"},
    {".htm", "text/html"},
    {".js", "text/javascript"},
    {".css", "text/css"},
    {".gif", "image/gif"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpg"},
    {".png", "image/png"},
    {".svg", "image/svg+xml"},
    {".webp", "image/webp"},
    {".tiff", "image/tiff"},
    {".ico", "image/x-icon"},
    {".wav", "audio/wav"},
    {".ogg", "audio/ogg"},
    {".mp3", "audio/mpeg"},
    {".webm", "video/webm"},
    {".json", "application/json"},
    {".exe", "application/octet-stream"},
    {".bin", "application/octet-stream"},
    {".class", "application/octet-stream"},
};

inline static const std::string default_mime("text/plain");

inline std::string get_mime(const std::string &filename) {
  auto pos = filename.rfind('.');
  if (pos == std::string::npos) {
    return default_mime;
  }
  auto suffix = filename.substr(pos);
  std::transform(suffix.begin(), suffix.end(), suffix.begin(), [](char c) { return isupper(c) ? c - 'A' + 'a' : c; });
  if (auto it = mime_map.find(suffix); it == mime_map.end()) {
    return default_mime;
  } else {
    return it->second;
  }
}

#endif //LILY_SRC_NET_HTTP_MIME_H_
