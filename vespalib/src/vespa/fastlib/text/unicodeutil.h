// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * Unicode utilities.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include <vespa/vespalib/text/lowercase.h>

/** ucs4_t is the type of the 4-byte UCS4 characters */
using ucs4_t = uint32_t;

/**
 * Utility class for unicode character handling.
 * Used to examine properties of unicode characters, and
 * provide fast conversion methods between often used encodings.
 */
class Fast_UnicodeUtil final {
private:
    // The bit vector generated from linguistics module, see
    // linguistics/src/test/java/com/yahoo/language/process/GenWordCharsBitVector.java
    static constexpr int maxCodeBlocks = 0x324;
    static unsigned long _wordCharBits[maxCodeBlocks * 0x100 / 64];

public:
    /** Indicates an invalid UTF-8 character sequence. */
    enum { _BadUTF8Char = 0xfffffffeu };

    /**
     * Test for word character. Characters with certain unicode properties
     * are recognized as word characters. In addition to this, all
     * characters with the custom _FASTWordProp is regarded as a word
     * character. The previous range in _privateUseProp is included
     * in the _FASTWordProp set of ranges.
     * @param testchar the UCS4 character to test.
     * @return true if testchar is a word character, i.e. if it has
     * one or more of the properties alphabetic, ideographic,
     * combining char, decimal digit char, private use, extender.
     */
    static bool IsWordChar(ucs4_t testchar) noexcept {
        if (testchar < (maxCodeBlocks * 0x100)) {
            unsigned int entry = (testchar >> 6);
            unsigned int bitnum = (testchar & 63);
            unsigned long mask = 1ul << bitnum;
            unsigned long bits = _wordCharBits[entry];
            return (bits & mask) != 0;
        }
        return false;
    }

    /**
     * Get the next UCS4 character from an UTF-8 string buffer.
     * Modify the src pointer to allow future calls.
     * @param src The address of a pointer to the current position
     *            in the UTF-8 string.
     * @param length The maximum allowed length of the byte sequence.
     *               -1 means no check.
     * @return The next UCS4 character, or _BadUTF8Char if the
     *         next character is invalid.
     */
    static ucs4_t GetUTF8Char(const unsigned char *& src) noexcept;
    static ucs4_t GetUTF8Char(const char *& src) noexcept {
        const unsigned char *temp = reinterpret_cast<const unsigned char *>(src);
        ucs4_t res = GetUTF8Char(temp);
        src = reinterpret_cast<const char *>(temp);
        return res;
    }

    /**
     * Put an UCS4 character into a buffer as an UTF-8 representation.
     * @param dst The destination buffer.
     * @param i The UCS4 character.
     * @return Pointer to the next position in dst after the putted byte(s).
     */
    static char *utf8cput(char *dst, ucs4_t i) noexcept {
        if (i < 128)
            *dst++ = i;
        else if (i < 0x800) {
            *dst++ = (i >> 6) | 0xc0;
            *dst++ = (i & 63) | 0x80;
        } else if (i < 0x10000) {
            *dst++ = (i >> 12) | 0xe0;
            *dst++ = ((i >> 6) & 63) | 0x80;
            *dst++ = (i & 63) | 0x80;
        } else if (i < 0x200000) {
            *dst++ = (i >> 18) | 0xf0;
            *dst++ = ((i >> 12) & 63) | 0x80;
            *dst++ = ((i >> 6) & 63) | 0x80;
            *dst++ = (i & 63) | 0x80;
        } else if (i < 0x4000000) {
            *dst++ = (i >> 24) | 0xf8;
            *dst++ = ((i >> 18) & 63) | 0x80;
            *dst++ = ((i >> 12) & 63) | 0x80;
            *dst++ = ((i >> 6) & 63) | 0x80;
            *dst++ = (i & 63) | 0x80;
        } else {
            *dst++ = (i >> 30) | 0xfc;
            *dst++ = ((i >> 24) & 63) | 0x80;
            *dst++ = ((i >> 18) & 63) | 0x80;
            *dst++ = ((i >> 12) & 63) | 0x80;
            *dst++ = ((i >> 6) & 63) | 0x80;
            *dst++ = (i & 63) | 0x80;
        }
        return dst;
    }

    /**
     * Copy an UTF-8 string into an UCS4 string.
     * @param dst The UCS4 destination buffer.
     * @param src The UTF-8 source buffer.
     * @return A pointer to the destination string.
     */
    static ucs4_t *ucs4copy(ucs4_t *dst, const char *src) noexcept;

    /**
     * Get the length of the UTF-8 representation of an UCS4 character.
     * @param i The UCS4 character.
     * @return The number of bytes required for the UTF-8 representation.
     */
    static size_t utf8clen(ucs4_t i) noexcept {
        if (i < 128)
            return 1;
        else if (i < 0x800)
            return 2;
        else if (i < 0x10000)
            return 3;
        else if (i < 0x200000)
            return 4;
        else if (i < 0x4000000)
            return 5;
        else
            return 6;
    }

    /**
     * Lowercase an UCS4 character.
     * @param testchar The character to lowercase.
     * @return The lowercase of the input, if defined. Else the input character.
     */
    static ucs4_t ToLower(ucs4_t testchar) noexcept
    {
        return vespalib::LowerCase::convert(testchar);
    }

    /** Move forwards or backwards a number of characters within an UTF8 buffer
     * Modify pos to yield new position if possible
     * @param start A pointer to the start of the UTF8 buffer
     * @param length The length of the UTF8 buffer
     * @param pos A pointer to the current position within the UTF8 buffer,
     *            updated to reflect new position upon return
     * @param offset An offset (+/-) in number of UTF8 characters.
     *        Offset 0 means move to the start of the current character.
     * @return Number of bytes moved, or -1 if out of range
     */
    static int UTF8move(unsigned const char* start, size_t length,
                        unsigned const char*& pos, off_t offset) noexcept;

    /**
     * Find the number of characters in an UCS4 string.
     * @param str The UCS4 string.
     * @return The number of characters.
     */
    static size_t ucs4strlen(const ucs4_t *str) noexcept;

    /**
     * Convert UCS4 to UTF-8, bounded by max lengths.
     * @param dst The destination buffer for the UTF-8 string.
     * @param src The source UCS4 string.
     * @param maxdst The maximum number of bytes to put into dst.
     * @param maxsrc The maximum number of characters to convert from src.
     * @return A pointer to the destination.
     */
    static char *utf8ncopy(char *dst, const ucs4_t *src, int maxdst, int maxsrc) noexcept;


    /**
     * Compare an UTF-8 string to a UCS4 string, analogous to strcmp(3).
     * @param s1 The UTF-8 string.
     * @param s2 The UCS4 string.
     * @return An integer less than, equal to, or greater than zero,
     *        if s1 is, respectively, less than, matching, or greater than s2.
     * NB Only used in local test
     */
    static int utf8cmp(const char *s1, const ucs4_t *s2) noexcept;

    /**
     * Get the next UCS4 character from an UTF-8 string buffer.
     * We assume that the first character in the UTF-8 string is >= 0x80 (non-ascii).
     * Modify the src pointer to allow future calls.
     * @param src The address of a pointer to the current position
     *            in the UTF-8 string.
     * @return The next UCS4 character, or _BadUTF8Char if the
     *         next character is invalid.
     */
    static ucs4_t GetUTF8CharNonAscii(unsigned const char *&src) noexcept;

    // this is really an alias of the above function
    static ucs4_t GetUTF8CharNonAscii(const char *&src) noexcept {
        unsigned const char *temp = reinterpret_cast<unsigned const char *>(src);
        ucs4_t res = GetUTF8CharNonAscii(temp);
        src = reinterpret_cast<const char *>(temp);
        return res;
    }
};
