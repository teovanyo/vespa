// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "version.h"
#include <vespa/vespalib/text/stringtokenizer.h>
#include <vespa/vespalib/util/exceptions.h>
#include <vespa/vespalib/stllike/asciistream.h>
#include <cctype>
#include <climits>

namespace vespalib {

Version::Version(int major, int minor, int micro, const string & qualifier)
    : _major(major),
      _minor(minor),
      _micro(micro),
      _qualifier(qualifier),
      _stringValue(),
      _abbreviatedStringValue()
{
    initialize();
}

Version::Version(const Version &) = default;
Version & Version::operator = (const Version &) = default;
Version::~Version() = default;

void
Version::initialize()
{
    asciistream buf;
    if (_qualifier != "") {
        buf << _major << "." << _minor << "." << _micro << "." << _qualifier;
    } else if (_micro > 0) {
        buf << _major << "." << _minor << "." << _micro;
    } else if (_minor > 0) {
        buf << _major << "." << _minor;
    } else if (_major > 0) {
        buf << _major;
    }
    _abbreviatedStringValue = buf.view();

    buf.clear();
    if (_qualifier != "") {
        buf << _major << "." << _minor << "." << _micro << "." << _qualifier;
    } else {
        buf << _major << "." << _minor << "." << _micro;
    }
    _stringValue = buf.view();

    if ((_major < 0) || (_minor < 0) || (_micro < 0) || !_qualifier.empty()) {
        verifySanity();
    }
}

void
Version::verifySanity()
{
    if (_major < 0)
        throw IllegalArgumentException("Negative major in " + _stringValue);

    if (_minor < 0)
        throw IllegalArgumentException("Negative minor in " + _stringValue);

    if (_micro < 0)
        throw IllegalArgumentException("Negative micro in " + _stringValue);

    if (_qualifier != "") {
        for (size_t i = 0; i < _qualifier.length(); i++) {
            unsigned char c = _qualifier[i];
            if (! std::isalnum(c)) {
                throw IllegalArgumentException("Error in " + _stringValue + ": Invalid character in qualifier");
            }
        }
    }
}

// Precondition: input.empty() == false
static int parseInteger(std::string_view input) __attribute__((noinline));
static int parseInteger(std::string_view input)
{
    const char *s = input.data();
    unsigned char firstDigit = s[0];
    if (!std::isdigit(firstDigit))
        throw IllegalArgumentException("integer must start with a digit");
    char *ep;
    long ret = strtol(s, &ep, 10);
    if (ret > INT_MAX || ret < 0) {
        throw IllegalArgumentException("integer out of range");
    }
    if (s + input.size() != ep ) {
        throw IllegalArgumentException("extra characters after integer");
    }
    return ret;
}


Version::Version(const string & versionString)
    : _major(0),
      _minor(0),
      _micro(0),
      _qualifier(),
      _stringValue(),
      _abbreviatedStringValue()
{
    if (!versionString.empty()) {
        StringTokenizer components(versionString, ".", ""); // Split on dot

        if (components.size() > 0) {
            _major = parseInteger(components[0]);
        }
        if (components.size() > 1) {
            _minor = parseInteger(components[1]);
        }
        if (components.size() > 2) {
            _micro = parseInteger(components[2]);
        }
        if (components.size() > 3) {
            _qualifier = components[3];
        }
        if (components.size() > 4) {
            throw IllegalArgumentException("too many dot-separated components in version string");
        }
    }
    initialize();
}

bool
Version::equals(const Version& other) const
{
    if (_major != other._major) return false;
    if (_minor != other._minor) return false;
    if (_micro != other._micro) return false;
    if (_qualifier != other._qualifier) return false;

    return true;
}

int
Version::compareTo(const Version& other) const
{
    int result = getMajor() - other.getMajor();
    if (result != 0) return result;

    result = getMinor() - other.getMinor();
    if (result != 0) return result;

    result = getMicro() - other.getMicro();
    if (result != 0) return result;

    return getQualifier().compare(other.getQualifier());
}

} // namespace vespalib
