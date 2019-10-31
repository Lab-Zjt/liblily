#include "../common/common.h"

interface Reader {
  R<ssize_t, Error> Read(lily::span<char> buf);
}

interface Writer {
  R<ssize_t, Error> Write(lily::span<char> buf);
}

interface ReadWriter extends Reader, Writer {}

interface Closer {
  Error Close();
}

interface Seeker {
  R<ssize_t, Error> Seek(ssize_t offset, int whence);
}

interface ReadSeeker extends Reader, Seeker {}

interface WriteSeeker extends Writer, Seeker {}

interface ReadWriteSeeker extends Reader, Writer, Seeker {}

interface ReadCloser extends Reader, Closer {}

interface WriteCloser extends Writer, Closer {}

interface ReadWriteCloser extends Reader, Writer, Closer {}

interface ReaderFrom {
  R<ssize_t, Error> ReadFrom(const Reader& reader);
}

interface WriterTo {
  R<ssize_t, Error> WriteTo(const Writer& writer);
}