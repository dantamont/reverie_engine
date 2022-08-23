/*

Note, the below code has been modified to better interface with Grand Blue, 
and is not representative of the original source.

*/

/*
The MIT License (MIT)

Copyright (c) 2012-2018 Syoyo Fujita and many contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditFVertexions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//
// version 2.0.0 : Add new object oriented API. 1.x API is still provided.
//                 * Support line primitive.
//                 * Support points primitive.
// version 1.4.0 : Modifed parseTextureNameAndOption API
// version 1.3.1 : Make parseTextureNameAndOption API public
// version 1.3.0 : Separate warning and error message(breaking API of LoadObj)
// version 1.2.3 : Added color space extension('-colorspace') to tex opts.
// version 1.2.2 : Parse multiple group names.
// version 1.2.1 : Added initial support for line('l') primitive(PR #178)
// version 1.2.0 : Hardened implementation(#175)
// version 1.1.1 : Support smoothing groups(#162)
// version 1.1.0 : Support parsing vertex color(#144)
// version 1.0.8 : Fix parsing `g` tag just after `usemtl`(#138)
// version 1.0.7 : Support multiple tex options(#126)
// version 1.0.6 : Add TINYOBJLOADER_USE_DOUBLE option(#124)
// version 1.0.5 : Ignore `Tr` when `d` exists in MTL(#43)
// version 1.0.4 : Support multiple filenames for 'mtllib'(#112)
// version 1.0.3 : Support parsing texture options(#85)
// version 1.0.2 : Improve parsing speed by about a factor of 2 for large
// files(#105)
// version 1.0.1 : Fixes a shape is lost if obj ends with a 'usemtl'(#104)
// version 1.0.0 : Change data structure. Change license from BSD to MIT.
//

//
// Use this in *one* .cc
   #define TINYOBJLOADER_IMPLEMENTATION
//   #include "tiny_obj_loader.h"
//

#ifndef TINY_OBJ_LOADER_H_
#define TINY_OBJ_LOADER_H_

#include <map>
#include <string>

#include "fortress/types/GString.h"
#include "fortress/containers/math/GVector.h"
#include "core/rendering/geometry/GVertexData.h"
#include "core/rendering/materials/GMaterial.h"

namespace tinyobj {

#ifdef __clang__
#pragma clang diagnostic push
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#pragma clang diagnostic ignored "-Wpadded"

#endif


// https://en.wikipedia.org/wiki/Wavefront_.obj_file says ...
//
//  -blendu on | off                       # set horizontal texture blending
//  (default on)
//  -blendv on | off                       # set vertical texture blending
//  (default on)
//  -boost real_value                      # boost mip-map sharpness
//  -mm base_value gain_value              # modify texture map values (default
//  0 1)
//                                         #     base_value = brightness,
//                                         gain_value = contrast
//  -o u [v [w]]                           # Origin offset             (default
//  0 0 0)
//  -s u [v [w]]                           # Scale                     (default
//  1 1 1)
//  -t u [v [w]]                           # Turbulence                (default
//  0 0 0)
//  -texres resolution                     # texture resolution to create
//  -clamp on | off                        # only render texels in the clamped
//  0-1 range (default off)
//                                         #   When unclamped, textures are
//                                         repeated across a surface,
//                                         #   when clamped, only texels which
//                                         fall within the 0-1
//                                         #   range are rendered.
//  -bm mult_value                         # bump multiplier (for bump maps
//  only)
//
//  -imfchan r | g | b | m | l | z         # specifies which channel of the file
//  is used to
//                                         # create a scalar or bump texture.
//                                         r:red, g:green,
//                                         # b:blue, m:matte, l:luminance,
//                                         z:z-depth..
//                                         # (the default for bump is 'l' and
//                                         for decal is 'm')
//  bump -imfchan r bumpmap.tga            # says to use the red channel of
//  bumpmap.tga as the bumpmap
//
// For reflection maps...
//
//   -type sphere                           # specifies a sphere for a "refl"
//   reflection map
//   -type cube_top    | cube_bottom |      # when using a cube map, the texture
//   file for each
//         cube_front  | cube_back   |      # side of the cube is specified
//         separately
//         cube_left   | cube_right
//
// TinyObjLoader extension.
//
//   -colorspace SPACE                      # Color space of the texture. e.g.
//   'sRGB` or 'linear'
//


/////////////////////////////////////////////////////////////////////////////////////////////
struct tag_t {
    std::string name;

    std::vector<int> intValues;
    std::vector<Real_t> floatValues;
    std::vector<std::string> stringValues;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Index struct to support different indices for vtx/normal/texcoord.
// -1 means not used.
struct index_t {
    int m_vertexIndex;
    int m_normalIndex;
    int m_texCoordIndex;
};

/////////////////////////////////////////////////////////////////////////////////////////////
struct mesh_t {
    std::vector<index_t> m_indices;
    std::vector<unsigned char>
        m_numFaceVertices;          // The number of vertices per
                                    // face. 3 = triangle, 4 = quad,
                                    // ... Up to 255 vertices per face.
    std::vector<int> m_materialIDs;  // per-face material ID
    std::vector<unsigned int> m_smoothingGroupIDs;  // per-face smoothing group
                                                    // ID(0 = off. positive value
                                                    // = group id)
    std::vector<tag_t> tags;                        // SubD tag
};
/////////////////////////////////////////////////////////////////////////////////////////////

struct lines_t {
    // Linear flattened indices.
    std::vector<index_t> indices;        // indices for vertices(poly lines)
    std::vector<int> num_line_vertices;  // The number of vertices per line.
};
/////////////////////////////////////////////////////////////////////////////////////////////
struct points_t {
    std::vector<index_t> indices;  // indices for points
};
/////////////////////////////////////////////////////////////////////////////////////////////
struct shape_t {
    std::string m_name;
    mesh_t m_mesh;
    lines_t m_lines;
    points_t m_points;
};

/////////////////////////////////////////////////////////////////////////////////////////////
class MaterialReader {
public:
    MaterialReader() {}
    virtual ~MaterialReader();

    virtual bool operator()(const std::string &matId,
        std::vector<rev::MaterialData> *materials,
        std::map<std::string, int> *matMap, std::string *warn,
        std::string *err) = 0;
};

///
/// Read .mtl from a file.
///
class MaterialFileReader : public MaterialReader {
public:
    explicit MaterialFileReader(const std::string &mtl_basedir)
        : m_mtlBaseDir(mtl_basedir) {}
    virtual ~MaterialFileReader() {}
    virtual bool operator()(const std::string &matId,
        std::vector<rev::MaterialData> *materials,
        std::map<std::string, int> *matMap, std::string *warn,
        std::string *err);

private:
    std::string m_mtlBaseDir;
};

///
/// Read .mtl from a stream.
///
class MaterialStreamReader : public MaterialReader {
public:
    explicit MaterialStreamReader(std::istream &inStream)
        : m_inStream(inStream) {}
    virtual ~MaterialStreamReader() {}
    virtual bool operator()(const std::string &matId,
        std::vector<rev::MaterialData> *materials,
        std::map<std::string, int> *matMap, std::string *warn,
        std::string *err);

private:
    std::istream &m_inStream;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// v2 API
struct ObjReaderConfig {
    bool m_triangulate;

    /// Parse vertex color.
    /// If vertex color is not present, its filled with default value.
    /// false = no vertex color
    /// This will increase memory of parsed .obj
    bool m_parseVertexColor;

    ///
    /// Search path to .mtl file.
    /// Default = "" = search from the same directory of .obj file.
    /// Valid only when loading .obj from a file.
    ///
    std::string m_mtlSearchPath;

    ObjReaderConfig() : 
        m_triangulate(true), 
        m_parseVertexColor(true) {}
};

/// ==>>========= Legacy v1 API =============================================

/// Loads .obj from a file.
/// 'attrib', 'shapes' and 'materials' will be filled with parsed shape data
/// 'shapes' will be filled with parsed shape data
/// Returns true when loading .obj become success.
/// Returns warning message into `warn`, and error message into `err`
/// 'mtl_basedir' is optional, and used for base directory for .mtl file.
/// In default(`NULL'), .mtl file is searched from an application's working
/// directory.
/// 'triangulate' is optional, and used whether triangulate polygon face in .obj
/// or not.
/// Option 'default_vcols_fallback' specifies whether vertex colors should
/// always be defined, even if no colors are given (fallback to white).
bool loadObj(rev::VertexAttributes *attrib, std::vector<shape_t> *shapes,
    std::vector<rev::MaterialData> *materials, std::string *warn,
    std::string *err, const char *filename,
    const char *mtl_basedir = NULL, bool triangulate = true,
    bool default_vcols_fallback = true);

/// Loads object from a std::istream, uses `readMatFn` to retrieve
/// std::istream for materials.
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
bool loadObj(rev::VertexAttributes *attrib, std::vector<shape_t> *shapes,
    std::vector<rev::MaterialData> *materials, std::string *warn,
    std::string *err, std::istream *inStream,
    MaterialReader *readMatFn = NULL, bool triangulate = true,
    bool default_vcols_fallback = true);

/// Loads materials into std::map
void loadMtl(std::map<std::string, int> *material_map,
    std::vector<rev::MaterialData> *materials, std::istream *inStream,
    std::string *warning, std::string *err);

///
/// Parse texture name and texture option for custom texture parameter through
/// material::unknown_parameter
///
/// @param[out] texname Parsed texture name
/// @param[out] texData Parsed texData
/// @param[in] linebuf Input string
///
bool parseTextureNameAndOption(rev::GString *texname, rev::TextureData *texData,
    const char *linebuf);

/// =<<========== Legacy v1 API =============================================

}  // namespace tinyobj

#endif  // TINY_OBJ_LOADER_H_


#ifdef TINYOBJLOADER_IMPLEMENTATION
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

#include <fstream>
#include <sstream>

namespace tinyobj {

MaterialReader::~MaterialReader() {}

struct vertex_index_t {
    int m_v_idx, m_vt_idx, m_vn_idx;
    vertex_index_t() : m_v_idx(-1), m_vt_idx(-1), m_vn_idx(-1) {}
    explicit vertex_index_t(int idx) : m_v_idx(idx), m_vt_idx(idx), m_vn_idx(idx) {}
    vertex_index_t(int vidx, int vtidx, int vnidx)
        : m_v_idx(vidx), m_vt_idx(vtidx), m_vn_idx(vnidx) {}
};

// Internal data structure for face representation
// index + smoothing group.
struct face_t {
    unsigned int
        smoothing_group_id;  // smoothing group id. 0 = smoothing groupd is off.
    int pad_;
    std::vector<vertex_index_t> vertex_indices;  // face vertex indices.

    face_t() : smoothing_group_id(0), pad_(0) {}
};

// Internal data structure for line representation
struct __line_t {
    // l v1/vt1 v2/vt2 ...
    // In the specification, line primitive does not have normal index, but
    // TinyObjLoader allows it
    std::vector<vertex_index_t> vertex_indices;
};

// Internal data structure for points representation
struct __points_t {
    // p v1 v2 ...
    // In the specification, point primitive does not have normal index and
    // texture coord index, but TinyObjLoader allows it.
    std::vector<vertex_index_t> vertex_indices;
};

struct tag_sizes {
    tag_sizes() : num_ints(0), num_reals(0), num_strings(0) {}
    int num_ints;
    int num_reals;
    int num_strings;
};

struct obj_shape {
    std::vector<Real_t> v;
    std::vector<Real_t> vn;
    std::vector<Real_t> vt;
};

//
// Manages group of primitives(face, line, points, ...)
struct PrimitiveGroup {
    std::vector<face_t> faceGroup;
    std::vector<__line_t> lineGroup;
    std::vector<__points_t> pointsGroup;

    void clear() {
        faceGroup.clear();
        lineGroup.clear();
        pointsGroup.clear();
    }

    bool isEmpty() const {
        return faceGroup.empty() && lineGroup.empty() && pointsGroup.empty();
    }

    // TODO(syoyo): bspline, surface, ...
};

// See
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
static std::istream &safeGetline(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    if (se) {
        for (;;) {
            int c = sb->sbumpc();
            switch (c) {
            case '\n':
                return is;
            case '\r':
                if (sb->sgetc() == '\n') sb->sbumpc();
                return is;
            case EOF:
                // Also handle the case when the last line has no line ending
                if (t.empty()) is.setstate(std::ios::eofbit);
                return is;
            default:
                t += static_cast<char>(c);
            }
        }
    }

    return is;
}

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

// Make index zero-base, and also support relative index.
static inline bool fixIndex(int idx, int n, int *ret) {
    if (!ret) {
        return false;
    }

    if (idx > 0) {
        (*ret) = idx - 1;
        return true;
    }

    if (idx == 0) {
        // zero is not allowed according to the spec.
        return false;
    }

    if (idx < 0) {
        (*ret) = n + idx;  // negative value = relative
        return true;
    }

    return false;  // never reach here.
}

static inline rev::GString parseString(const char **token) {
    rev::GString s;
    (*token) += strspn((*token), " \t");
    size_t e = strcspn((*token), " \t\r");
    s = rev::GString(std::string((*token), &(*token)[e]).c_str());
    (*token) += e;
    return s;
}

static inline int parseInt(const char **token) {
    (*token) += strspn((*token), " \t");
    int i = atoi((*token));
    (*token) += strcspn((*token), " \t\r");
    return i;
}

// Tries to parse a floating point number located at s.
//
// s_end should be a location in the string where reading should absolutely
// stop. For example at the end of the string, to prevent buffer overflows.
//
// Parses the following EBNF grammar:
//   sign    = "+" | "-" ;
//   END     = ? anything not in digit ?
//   digit   = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
//   integer = [sign] , digit , {digit} ;
//   decimal = integer , ["." , integer] ;
//   float   = ( decimal , END ) | ( decimal , ("E" | "e") , integer , END ) ;
//
//  Valid strings are for example:
//   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41   11e2
//
// If the parsing is a success, result is set to the parsed value and true
// is returned.
//
// The function is greedy and will parse until any of the following happens:
//  - a non-conforming character is encountered.
//  - s_end is reached.
//
// The following situations triggers a failure:
//  - s >= s_end.
//  - parse failure.
//
static bool tryParseDouble(const char *s, const char *s_end, double *result) {
    if (s >= s_end) {
        return false;
    }

    double mantissa = 0.0;
    // This exponent is base 2 rather than 10.
    // However the exponent we parse is supposed to be one of ten,
    // thus we must take care to convert the exponent/and or the
    // mantissa to a * 2^E, where a is the mantissa and E is the
    // exponent.
    // To get the final double we will use ldexp, it requires the
    // exponent to be in base 2.
    int exponent = 0;

    // NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
    // TO JUMP OVER DEFINITIONS.
    char sign = '+';
    char exp_sign = '+';
    char const *curr = s;

    // How many characters were read in a loop.
    int read = 0;
    // Tells whether a loop terminated due to reaching s_end.
    bool end_not_reached = false;
    bool leading_decimal_dots = false;

    /*
            BEGIN PARSING.
    */

    // Find out what sign we've got.
    if (*curr == '+' || *curr == '-') {
        sign = *curr;
        curr++;
        if ((curr != s_end) && (*curr == '.')) {
            // accept. Somethig like `.7e+2`, `-.5234`
            leading_decimal_dots = true;
        }
    }
    else if (IS_DIGIT(*curr)) { /* Pass through. */
    }
    else if (*curr == '.') {
        // accept. Somethig like `.7e+2`, `-.5234`
        leading_decimal_dots = true;
    }
    else {
        goto fail;
    }

    // Read the integer part.
    end_not_reached = (curr != s_end);
    if (!leading_decimal_dots) {
        while (end_not_reached && IS_DIGIT(*curr)) {
            mantissa *= 10;
            mantissa += static_cast<int>(*curr - 0x30);
            curr++;
            read++;
            end_not_reached = (curr != s_end);
        }

        // We must make sure we actually got something.
        if (read == 0) goto fail;
    }

    // We allow numbers of form "#", "###" etc.
    if (!end_not_reached) goto assemble;

    // Read the decimal part.
    if (*curr == '.') {
        curr++;
        read = 1;
        end_not_reached = (curr != s_end);
        while (end_not_reached && IS_DIGIT(*curr)) {
            static const double pow_lut[] = {
                1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
            };
            const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];

            // NOTE: Don't use powf here, it will absolutely murder precision.
            mantissa += static_cast<int>(*curr - 0x30) *
                (read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
            read++;
            curr++;
            end_not_reached = (curr != s_end);
        }
    }
    else if (*curr == 'e' || *curr == 'E') {
    }
    else {
        goto assemble;
    }

    if (!end_not_reached) goto assemble;

    // Read the exponent part.
    if (*curr == 'e' || *curr == 'E') {
        curr++;
        // Figure out if a sign is present and if it is.
        end_not_reached = (curr != s_end);
        if (end_not_reached && (*curr == '+' || *curr == '-')) {
            exp_sign = *curr;
            curr++;
        }
        else if (IS_DIGIT(*curr)) { /* Pass through. */
        }
        else {
            // Empty E is not allowed.
            goto fail;
        }

        read = 0;
        end_not_reached = (curr != s_end);
        while (end_not_reached && IS_DIGIT(*curr)) {
            exponent *= 10;
            exponent += static_cast<int>(*curr - 0x30);
            curr++;
            read++;
            end_not_reached = (curr != s_end);
        }
        exponent *= (exp_sign == '+' ? 1 : -1);
        if (read == 0) goto fail;
    }

assemble:
    *result = (sign == '+' ? 1 : -1) *
        (exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent)
            : mantissa);
    return true;
fail:
    return false;
}

static inline Real_t parseReal(const char **token, double default_value = 0.0) {
    (*token) += strspn((*token), " \t");
    const char *end = (*token) + strcspn((*token), " \t\r");
    double val = default_value;
    tryParseDouble((*token), end, &val);
    Real_t f = static_cast<Real_t>(val);
    (*token) = end;
    return f;
}

static inline bool parseReal(const char **token, Real_t *out) {
    (*token) += strspn((*token), " \t");
    const char *end = (*token) + strcspn((*token), " \t\r");
    double val;
    bool ret = tryParseDouble((*token), end, &val);
    if (ret) {
        Real_t f = static_cast<Real_t>(val);
        (*out) = f;
    }
    (*token) = end;
    return ret;
}

static inline void parseReal2(Real_t *x, Real_t *y, const char **token,
    const double default_x = 0.0,
    const double default_y = 0.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
}

static inline void parseReal3(Real_t *x, Real_t *y, Real_t *z,
    const char **token, const double default_x = 0.0,
    const double default_y = 0.0,
    const double default_z = 0.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
}

static inline void parseV(Real_t *x, Real_t *y, Real_t *z, Real_t *w,
    const char **token, const double default_x = 0.0,
    const double default_y = 0.0,
    const double default_z = 0.0,
    const double default_w = 1.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
    (*w) = parseReal(token, default_w);
}

// Extension: parse vertex with colors(6 items)
static inline bool parseVertexWithColor(Real_t *x, Real_t *y, Real_t *z,
    Real_t *r, Real_t *g, Real_t *b,
    const char **token,
    const double default_x = 0.0,
    const double default_y = 0.0,
    const double default_z = 0.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);

    const bool found_color =
        parseReal(token, r) && parseReal(token, g) && parseReal(token, b);

    if (!found_color) {
        (*r) = (*g) = (*b) = 1.0;
    }

    return found_color;
}

static inline bool parseOnOff(const char **token, bool default_value = true) {
    (*token) += strspn((*token), " \t");
    const char *end = (*token) + strcspn((*token), " \t\r");

    bool ret = default_value;
    if ((0 == strncmp((*token), "on", 2))) {
        ret = true;
    }
    else if ((0 == strncmp((*token), "off", 3))) {
        ret = false;
    }

    (*token) = end;
    return ret;
}

static inline rev::TextureModelType parseTextureType(
    const char **token, rev::TextureModelType default_value = rev::kTextureModelType_None) {
    (*token) += strspn((*token), " \t");
    const char *end = (*token) + strcspn((*token), " \t\r");
    rev::TextureModelType ty = default_value;

    if ((0 == strncmp((*token), "cube_top", strlen("cube_top")))) {
        ty = rev::kTextureModelType_Cube_Top;
    }
    else if ((0 == strncmp((*token), "cube_bottom", strlen("cube_bottom")))) {
        ty = rev::kTextureModelType_Cube_Bottom;
    }
    else if ((0 == strncmp((*token), "cube_left", strlen("cube_left")))) {
        ty = rev::kTextureModelType_Cube_Left;
    }
    else if ((0 == strncmp((*token), "cube_right", strlen("cube_right")))) {
        ty = rev::kTextureModelType_Cube_Right;
    }
    else if ((0 == strncmp((*token), "cube_front", strlen("cube_front")))) {
        ty = rev::kTextureModelType_Cube_Front;
    }
    else if ((0 == strncmp((*token), "cube_back", strlen("cube_back")))) {
        ty = rev::kTextureModelType_Cube_Back;
    }
    else if ((0 == strncmp((*token), "sphere", strlen("sphere")))) {
        ty = rev::kTextureModelType_Sphere;
    }

    (*token) = end;
    return ty;
}

static tag_sizes parseTagTriple(const char **token) {
    tag_sizes ts;

    (*token) += strspn((*token), " \t");
    ts.num_ints = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return ts;
    }

    (*token)++;  // Skip '/'

    (*token) += strspn((*token), " \t");
    ts.num_reals = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return ts;
    }
    (*token)++;  // Skip '/'

    ts.num_strings = parseInt(token);

    return ts;
}

// Parse triples with index offsets: i, i/j/k, i//k, i/j
static bool parseTriple(const char **token, int vsize, int vnsize, int vtsize,
    vertex_index_t *ret) {
    if (!ret) {
        return false;
    }

    vertex_index_t vi(-1);

    if (!fixIndex(atoi((*token)), vsize, &(vi.m_v_idx))) {
        return false;
    }

    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        (*ret) = vi;
        return true;
    }
    (*token)++;

    // i//k
    if ((*token)[0] == '/') {
        (*token)++;
        if (!fixIndex(atoi((*token)), vnsize, &(vi.m_vn_idx))) {
            return false;
        }
        (*token) += strcspn((*token), "/ \t\r");
        (*ret) = vi;
        return true;
    }

    // i/j/k or i/j
    if (!fixIndex(atoi((*token)), vtsize, &(vi.m_vt_idx))) {
        return false;
    }

    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        (*ret) = vi;
        return true;
    }

    // i/j/k
    (*token)++;  // skip '/'
    if (!fixIndex(atoi((*token)), vnsize, &(vi.m_vn_idx))) {
        return false;
    }
    (*token) += strcspn((*token), "/ \t\r");

    (*ret) = vi;

    return true;
}

// Parse raw triples: i, i/j/k, i//k, i/j
static vertex_index_t parseRawTriple(const char **token) {
    vertex_index_t vi(static_cast<int>(0));  // 0 is an invalid index in OBJ

    vi.m_v_idx = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }
    (*token)++;

    // i//k
    if ((*token)[0] == '/') {
        (*token)++;
        vi.m_vn_idx = atoi((*token));
        (*token) += strcspn((*token), "/ \t\r");
        return vi;
    }

    // i/j/k or i/j
    vi.m_vt_idx = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }

    // i/j/k
    (*token)++;  // skip '/'
    vi.m_vn_idx = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
}

bool parseTextureNameAndOption(rev::GString *texname, rev::TextureData *texData,
    const char *linebuf) {
    // @todo { write more robust lexer and parser. }
    bool found_texname = false;
    rev::GString texture_name;

    const char *token = linebuf;  // Assume line ends with NULL

    while (!IS_NEW_LINE((*token))) {
        token += strspn(token, " \t");  // skip space
        if ((0 == strncmp(token, "-blendu", 7)) && IS_SPACE((token[7]))) {
            token += 8;
            texData->m_properties.m_blendu = parseOnOff(&token, /* default */ true);
        }
        else if ((0 == strncmp(token, "-blendv", 7)) && IS_SPACE((token[7]))) {
            token += 8;
            texData->m_properties.m_blendv = parseOnOff(&token, /* default */ true);
        }
        else if ((0 == strncmp(token, "-clamp", 6)) && IS_SPACE((token[6]))) {
            token += 7;
            texData->m_properties.m_clamp = parseOnOff(&token, /* default */ true);
        }
        else if ((0 == strncmp(token, "-boost", 6)) && IS_SPACE((token[6]))) {
            token += 7;
            texData->m_properties.m_sharpness = parseReal(&token, 1.0);
        }
        else if ((0 == strncmp(token, "-bm", 3)) && IS_SPACE((token[3]))) {
            token += 4;
            texData->m_properties.m_bumpMultiplier = parseReal(&token, 1.0);
        }
        else if ((0 == strncmp(token, "-o", 2)) && IS_SPACE((token[2]))) {
            token += 3;
            parseReal3(&(texData->m_properties.m_originOffset[0]), &(texData->m_properties.m_originOffset[1]),
                &(texData->m_properties.m_originOffset[2]), &token);
        }
        else if ((0 == strncmp(token, "-s", 2)) && IS_SPACE((token[2]))) {
            token += 3;
            parseReal3(&(texData->m_properties.m_scale[0]), &(texData->m_properties.m_scale[1]), &(texData->m_properties.m_scale[2]),
                &token, 1.0, 1.0, 1.0);
        }
        else if ((0 == strncmp(token, "-t", 2)) && IS_SPACE((token[2]))) {
            token += 3;
            parseReal3(&(texData->m_properties.m_turbulence[0]), &(texData->m_properties.m_turbulence[1]),
                &(texData->m_properties.m_turbulence[2]), &token);
        }
        else if ((0 == strncmp(token, "-type", 5)) && IS_SPACE((token[5]))) {
            token += 5;
            texData->m_properties.m_textureModelType = parseTextureType((&token), rev::kTextureModelType_None);
        }
        else if ((0 == strncmp(token, "-imfchan", 8)) && IS_SPACE((token[8]))) {
            token += 9;
            token += strspn(token, " \t");
            const char *end = token + strcspn(token, " \t\r");
            if ((end - token) == 1) {  // Assume one char for -imfchan
                texData->m_properties.m_imfChannel = (*token);
            }
            token = end;
        }
        else if ((0 == strncmp(token, "-mm", 3)) && IS_SPACE((token[3]))) {
            token += 4;
            parseReal2(&(texData->m_properties.m_brightness), &(texData->m_properties.m_contrast), &token, 0.0, 1.0);
        }
        else if ((0 == strncmp(token, "-colorspace", 11)) &&
            IS_SPACE((token[11]))) {
            token += 12;
            // FIXME: Removed, due to laziness, and lack of desire to store a string
            //texData->m_properties.m_colorSpace = parseString(&token);
        }
        else {
            // Assume texture filename
#if 0
            size_t len = strcspn(token, " \t\r");  // untile next space
            texture_name = std::string(token, token + len);
            token += len;

            token += strspn(token, " \t");  // skip space
#else
      // Read filename until line end to parse filename containing whitespace
      // TODO(syoyo): Support parsing texture option flag after the filename.
            texture_name = rev::GString(token);
            token += texture_name.length();
#endif

            found_texname = true;
        }
    }

    if (found_texname) {
        (*texname) = texture_name;
        return true;
    }
    else {
        return false;
    }
}


// code from https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
template <typename T>
static int pnpoly(int nvert, T *vertx, T *verty, T testx, T testy) {
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx <
            (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) +
                vertx[i]))
            c = !c;
    }
    return c;
}

// TODO(syoyo): refactor function.
static bool exportGroupsToShape(shape_t *shape, const PrimitiveGroup &prim_group,
    const std::vector<tag_t> &tags,
    const int material_id, const std::string &name,
    bool triangulate,
    const std::vector<rev::Vector3> &v) {
    if (prim_group.isEmpty()) {
        return false;
    }

    shape->m_name = name;

    // polygon
    if (!prim_group.faceGroup.empty()) {
        // Flatten vertices and indices
        for (size_t i = 0; i < prim_group.faceGroup.size(); i++) {
            const face_t &face = prim_group.faceGroup[i];

            size_t npolys = face.vertex_indices.size();

            if (npolys < 3) {
                // Face must have 3+ vertices.
                continue;
            }

            vertex_index_t i0 = face.vertex_indices[0];
            vertex_index_t i1(-1);
            vertex_index_t i2 = face.vertex_indices[1];

            if (triangulate) {
                // find the two axes to work in
                size_t axes[2] = { 1, 2 };
                for (size_t k = 0; k < npolys; ++k) {
                    i0 = face.vertex_indices[(k + 0) % npolys];
                    i1 = face.vertex_indices[(k + 1) % npolys];
                    i2 = face.vertex_indices[(k + 2) % npolys];
                    size_t vi0 = size_t(i0.m_v_idx);
                    size_t vi1 = size_t(i1.m_v_idx);
                    size_t vi2 = size_t(i2.m_v_idx);

                    if (((vi0 + 2) >= v.size()) || ((vi1 + 2) >= v.size()) ||
                        ((vi2 + 2) >= v.size())) {
                        // Invalid triangle.
                        // FIXME(syoyo): Is it ok to simply skip this invalid triangle?
                        continue;
                    }
                    Real_t v0x = v[vi0 + 0].x();
                    Real_t v0y = v[vi0 + 1].y();
                    Real_t v0z = v[vi0 + 2].z();
                    Real_t v1x = v[vi1 + 0].x();
                    Real_t v1y = v[vi1 + 1].y();
                    Real_t v1z = v[vi1 + 2].z();
                    Real_t v2x = v[vi2 + 0].x();
                    Real_t v2y = v[vi2 + 1].y();
                    Real_t v2z = v[vi2 + 2].z();
                    Real_t e0x = v1x - v0x;
                    Real_t e0y = v1y - v0y;
                    Real_t e0z = v1z - v0z;
                    Real_t e1x = v2x - v1x;
                    Real_t e1y = v2y - v1y;
                    Real_t e1z = v2z - v1z;
                    Real_t cx = std::fabs(e0y * e1z - e0z * e1y);
                    Real_t cy = std::fabs(e0z * e1x - e0x * e1z);
                    Real_t cz = std::fabs(e0x * e1y - e0y * e1x);
                    const Real_t epsilon = std::numeric_limits<Real_t>::epsilon();
                    if (cx > epsilon || cy > epsilon || cz > epsilon) {
                        // found a corner
                        if (cx > cy && cx > cz) {
                        }
                        else {
                            axes[0] = 0;
                            if (cz > cx && cz > cy) axes[1] = 1;
                        }
                        break;
                    }
                }

                Real_t area = 0;
                for (size_t k = 0; k < npolys; ++k) {
                    i0 = face.vertex_indices[(k + 0) % npolys];
                    i1 = face.vertex_indices[(k + 1) % npolys];
                    size_t vi0 = size_t(i0.m_v_idx);
                    size_t vi1 = size_t(i1.m_v_idx);
                    if (((vi0 + axes[0]) >= v.size()) ||
                        ((vi0 + axes[1]) >= v.size()) ||
                        ((vi1 + axes[0]) >= v.size()) ||
                        ((vi1 + axes[1]) >= v.size())) {
                        // Invalid index.
                        continue;
                    }
                    Real_t v0x = v[vi0][axes[0]];
                    Real_t v0y = v[vi0][axes[1]];
                    Real_t v1x = v[vi1][axes[0]];
                    Real_t v1y = v[vi1][axes[1]];
                    area += (v0x * v1y - v0y * v1x) * static_cast<Real_t>(0.5);
                }

                face_t remainingFace = face;  // copy
                size_t guess_vert = 0;
                vertex_index_t ind[3];
                Real_t vx[3];
                Real_t vy[3];

                // How many iterations can we do without decreasing the remaining
                // vertices.
                size_t remainingIterations = face.vertex_indices.size();
                size_t previousRemainingVertices = remainingFace.vertex_indices.size();

                while (remainingFace.vertex_indices.size() > 3 &&
                    remainingIterations > 0) {
                    npolys = remainingFace.vertex_indices.size();
                    if (guess_vert >= npolys) {
                        guess_vert -= npolys;
                    }

                    if (previousRemainingVertices != npolys) {
                        // The number of remaining vertices decreased. Reset counters.
                        previousRemainingVertices = npolys;
                        remainingIterations = npolys;
                    }
                    else {
                        // We didn't consume a vertex on previous iteration, reduce the
                        // available iterations.
                        remainingIterations--;
                    }

                    for (size_t k = 0; k < 3; k++) {
                        ind[k] = remainingFace.vertex_indices[(guess_vert + k) % npolys];
                        size_t vi = size_t(ind[k].m_v_idx);
                        if (((vi * 3 + axes[0]) >= v.size()) ||
                            ((vi * 3 + axes[1]) >= v.size())) {
                            // ???
                            vx[k] = static_cast<Real_t>(0.0);
                            vy[k] = static_cast<Real_t>(0.0);
                        }
                        else {
                            vx[k] = v[vi][axes[0]];
                            vy[k] = v[vi][axes[0]];
                        }
                    }
                    Real_t e0x = vx[1] - vx[0];
                    Real_t e0y = vy[1] - vy[0];
                    Real_t e1x = vx[2] - vx[1];
                    Real_t e1y = vy[2] - vy[1];
                    Real_t cross = e0x * e1y - e0y * e1x;
                    // if an internal angle
                    if (cross * area < static_cast<Real_t>(0.0)) {
                        guess_vert += 1;
                        continue;
                    }

                    // check all other verts in case they are inside this triangle
                    bool overlap = false;
                    for (size_t otherVert = 3; otherVert < npolys; ++otherVert) {
                        size_t idx = (guess_vert + otherVert) % npolys;

                        if (idx >= remainingFace.vertex_indices.size()) {
                            // ???
                            continue;
                        }

                        size_t ovi = size_t(remainingFace.vertex_indices[idx].m_v_idx);

                        if (((ovi * 3 + axes[0]) >= v.size()) ||
                            ((ovi * 3 + axes[1]) >= v.size())) {
                            // ???
                            continue;
                        }
                        Real_t tx = v[ovi][axes[0]];
                        Real_t ty = v[ovi][axes[1]];
                        if (pnpoly(3, vx, vy, tx, ty)) {
                            overlap = true;
                            break;
                        }
                    }

                    if (overlap) {
                        guess_vert += 1;
                        continue;
                    }

                    // this triangle is an ear
                    {
                        index_t idx0, idx1, idx2;
                        idx0.m_vertexIndex = ind[0].m_v_idx;
                        idx0.m_normalIndex = ind[0].m_vn_idx;
                        idx0.m_texCoordIndex = ind[0].m_vt_idx;
                        idx1.m_vertexIndex = ind[1].m_v_idx;
                        idx1.m_normalIndex = ind[1].m_vn_idx;
                        idx1.m_texCoordIndex = ind[1].m_vt_idx;
                        idx2.m_vertexIndex = ind[2].m_v_idx;
                        idx2.m_normalIndex = ind[2].m_vn_idx;
                        idx2.m_texCoordIndex = ind[2].m_vt_idx;

                        shape->m_mesh.m_indices.push_back(idx0);
                        shape->m_mesh.m_indices.push_back(idx1);
                        shape->m_mesh.m_indices.push_back(idx2);

                        shape->m_mesh.m_numFaceVertices.push_back(3);
                        shape->m_mesh.m_materialIDs.push_back(material_id);
                        shape->m_mesh.m_smoothingGroupIDs.push_back(face.smoothing_group_id);
                    }

                    // remove v1 from the list
                    size_t removed_vert_index = (guess_vert + 1) % npolys;
                    while (removed_vert_index + 1 < npolys) {
                        remainingFace.vertex_indices[removed_vert_index] =
                            remainingFace.vertex_indices[removed_vert_index + 1];
                        removed_vert_index += 1;
                    }
                    remainingFace.vertex_indices.pop_back();
                }

                if (remainingFace.vertex_indices.size() == 3) {
                    i0 = remainingFace.vertex_indices[0];
                    i1 = remainingFace.vertex_indices[1];
                    i2 = remainingFace.vertex_indices[2];
                    {
                        index_t idx0, idx1, idx2;
                        idx0.m_vertexIndex = i0.m_v_idx;
                        idx0.m_normalIndex = i0.m_vn_idx;
                        idx0.m_texCoordIndex = i0.m_vt_idx;
                        idx1.m_vertexIndex = i1.m_v_idx;
                        idx1.m_normalIndex = i1.m_vn_idx;
                        idx1.m_texCoordIndex = i1.m_vt_idx;
                        idx2.m_vertexIndex = i2.m_v_idx;
                        idx2.m_normalIndex = i2.m_vn_idx;
                        idx2.m_texCoordIndex = i2.m_vt_idx;

                        shape->m_mesh.m_indices.push_back(idx0);
                        shape->m_mesh.m_indices.push_back(idx1);
                        shape->m_mesh.m_indices.push_back(idx2);

                        shape->m_mesh.m_numFaceVertices.push_back(3);
                        shape->m_mesh.m_materialIDs.push_back(material_id);
                        shape->m_mesh.m_smoothingGroupIDs.push_back(face.smoothing_group_id);
                    }
                }
            }
            else {
                for (size_t k = 0; k < npolys; k++) {
                    index_t idx;
                    idx.m_vertexIndex = face.vertex_indices[k].m_v_idx;
                    idx.m_normalIndex = face.vertex_indices[k].m_vn_idx;
                    idx.m_texCoordIndex = face.vertex_indices[k].m_vt_idx;
                    shape->m_mesh.m_indices.push_back(idx);
                }

                shape->m_mesh.m_numFaceVertices.push_back(
                    static_cast<unsigned char>(npolys));
                shape->m_mesh.m_materialIDs.push_back(material_id);  // per face
                shape->m_mesh.m_smoothingGroupIDs.push_back(
                    face.smoothing_group_id);  // per face
            }
        }

        shape->m_mesh.tags = tags;
    }

    // line
    if (!prim_group.lineGroup.empty()) {
        // Flatten indices
        for (size_t i = 0; i < prim_group.lineGroup.size(); i++) {
            for (size_t j = 0; j < prim_group.lineGroup[i].vertex_indices.size();
                j++) {
                const vertex_index_t &vi = prim_group.lineGroup[i].vertex_indices[j];

                index_t idx;
                idx.m_vertexIndex = vi.m_v_idx;
                idx.m_normalIndex = vi.m_vn_idx;
                idx.m_texCoordIndex = vi.m_vt_idx;

                shape->m_lines.indices.push_back(idx);
            }

            shape->m_lines.num_line_vertices.push_back(
                int(prim_group.lineGroup[i].vertex_indices.size()));
        }
    }

    // points
    if (!prim_group.pointsGroup.empty()) {
        // Flatten & convert indices
        for (size_t i = 0; i < prim_group.pointsGroup.size(); i++) {
            for (size_t j = 0; j < prim_group.pointsGroup[i].vertex_indices.size();
                j++) {
                const vertex_index_t &vi = prim_group.pointsGroup[i].vertex_indices[j];

                index_t idx;
                idx.m_vertexIndex = vi.m_v_idx;
                idx.m_normalIndex = vi.m_vn_idx;
                idx.m_texCoordIndex = vi.m_vt_idx;

                shape->m_points.indices.push_back(idx);
            }
        }
    }

    return true;
}

// Split a string with specified delimiter character.
// http://stackoverflow.com/questions/236129/split-a-string-in-c
static void splitString(const std::string &s, char delim,
    std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

// Load material from inStream
void loadMtl(std::map<std::string, int> *material_map,
    std::vector<rev::MaterialData> *materials, std::istream *inStream,
    std::string *warning, std::string *err) {
    (void)err;

    // Create a default material anyway.
    rev::MaterialData material;

    // Issue 43. `d` wins against `Tr` since `Tr` is not in the MTL specification.
    bool has_d = false;
    bool has_tr = false;

    std::stringstream warn_ss;

    size_t line_no = 0;
    std::string linebuf;
    while (inStream->peek() != -1) {
        safeGetline(*inStream, linebuf);
        line_no++;

        // Trim trailing whitespace.
        if (linebuf.size() > 0) {
            linebuf = linebuf.substr(0, linebuf.find_last_not_of(" \t") + 1);
        }

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\n')
                linebuf.erase(linebuf.size() - 1);
        }
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\r')
                linebuf.erase(linebuf.size() - 1);
        }

        // Skip if empty line.
        if (linebuf.empty()) {
            continue;
        }

        // Skip leading space.
        const char *token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0') continue;  // empty line

        if (token[0] == '#') continue;  // comment line

        // new mtl
        if ((0 == strncmp(token, "newmtl", 6)) && IS_SPACE((token[6]))) {
            // flush previous material.
            if (!material.m_name.isEmpty()) {
                material_map->insert(std::pair<std::string, int>(
                    (std::string)material.m_name, static_cast<int>(materials->size())));
                material.m_id = static_cast<int>(materials->size());
                materials->push_back(material);
            }

            // initial temporary material
            material.initialize();

            has_d = false;
            has_tr = false;

            // set new mtl name
            token += 7;
            {
                std::stringstream sstr;
                sstr << token;
                material.m_name = sstr.str().c_str();
            }
            continue;
        }

        // ambient
        if (token[0] == 'K' && token[1] == 'a' && IS_SPACE((token[2]))) {
            token += 2;
            Real_t r, g, b;
            parseReal3(&r, &g, &b, &token);
            material.m_properties.m_ambient[0] = r;
            material.m_properties.m_ambient[1] = g;
            material.m_properties.m_ambient[2] = b;
            continue;
        }

        // diffuse
        if (token[0] == 'K' && token[1] == 'd' && IS_SPACE((token[2]))) {
            token += 2;
            Real_t r, g, b;
            parseReal3(&r, &g, &b, &token);
            material.m_properties.m_diffuse[0] = r;
            material.m_properties.m_diffuse[1] = g;
            material.m_properties.m_diffuse[2] = b;
            continue;
        }

        // specular
        if (token[0] == 'K' && token[1] == 's' && IS_SPACE((token[2]))) {
            token += 2;
            Real_t r, g, b;
            parseReal3(&r, &g, &b, &token);
            material.m_properties.m_specularity[0] = r;
            material.m_properties.m_specularity[1] = g;
            material.m_properties.m_specularity[2] = b;
            continue;
        }

        // transmittance
        if ((token[0] == 'K' && token[1] == 't' && IS_SPACE((token[2]))) ||
            (token[0] == 'T' && token[1] == 'f' && IS_SPACE((token[2])))) {
            token += 2;
            Real_t r, g, b;
            parseReal3(&r, &g, &b, &token);
            material.m_properties.m_transmittance[0] = r;
            material.m_properties.m_transmittance[1] = g;
            material.m_properties.m_transmittance[2] = b;
            continue;
        }

        // ior(index of refraction)
        if (token[0] == 'N' && token[1] == 'i' && IS_SPACE((token[2]))) {
            token += 2;
            material.m_properties.m_ior = parseReal(&token);
            continue;
        }

        // emission
        if (token[0] == 'K' && token[1] == 'e' && IS_SPACE(token[2])) {
            token += 2;
            Real_t r, g, b;
            parseReal3(&r, &g, &b, &token);
            material.m_properties.m_emission[0] = r;
            material.m_properties.m_emission[1] = g;
            material.m_properties.m_emission[2] = b;
            continue;
        }

        // shininess
        if (token[0] == 'N' && token[1] == 's' && IS_SPACE(token[2])) {
            token += 2;
            material.m_properties.m_shininess = parseReal(&token);
            continue;
        }

        // illum model
        if (0 == strncmp(token, "illum", 5) && IS_SPACE(token[5])) {
            token += 6;
            material.m_properties.m_illum = parseInt(&token);
            continue;
        }

        // dissolve
        if ((token[0] == 'd' && IS_SPACE(token[1]))) {
            token += 1;
            material.m_properties.m_dissolve = parseReal(&token);

            if (has_tr) {
                warn_ss << "Both `d` and `Tr` parameters defined for \""
                    << material.m_name
                    << "\". Use the value of `d` for dissolve (line " << line_no
                    << " in .mtl.)" << std::endl;
            }
            has_d = true;
            continue;
        }
        if (token[0] == 'T' && token[1] == 'r' && IS_SPACE(token[2])) {
            token += 2;
            if (has_d) {
                // `d` wins. Ignore `Tr` value.
                warn_ss << "Both `d` and `Tr` parameters defined for \""
                    << material.m_name
                    << "\". Use the value of `d` for dissolve (line " << line_no
                    << " in .mtl.)" << std::endl;
            }
            else {
                // We invert value of Tr(assume Tr is in range [0, 1])
                // NOTE: Interpretation of Tr is application(exporter) dependent. For
                // some application(e.g. 3ds max obj exporter), Tr = d(Issue 43)
                material.m_properties.m_dissolve = static_cast<Real_t>(1.0) - parseReal(&token);
            }
            has_tr = true;
            continue;
        }

        // PBR: roughness
        if (token[0] == 'P' && token[1] == 'r' && IS_SPACE(token[2])) {
            token += 2;
            material.m_properties.m_roughness = parseReal(&token);
            continue;
        }

        // PBR: metallic
        if (token[0] == 'P' && token[1] == 'm' && IS_SPACE(token[2])) {
            token += 2;
            material.m_properties.m_metallic = parseReal(&token);
            continue;
        }

        // PBR: sheen
        if (token[0] == 'P' && token[1] == 's' && IS_SPACE(token[2])) {
            token += 2;
            material.m_properties.m_sheen = parseReal(&token);
            continue;
        }

        // PBR: clearcoat thickness
        if (token[0] == 'P' && token[1] == 'c' && IS_SPACE(token[2])) {
            token += 2;
            material.m_properties.m_clearcoatThickness = parseReal(&token);
            continue;
        }

        // PBR: clearcoat roughness
        if ((0 == strncmp(token, "Pcr", 3)) && IS_SPACE(token[3])) {
            token += 4;
            material.m_properties.m_clearcoatRoughness = parseReal(&token);
            continue;
        }

        // PBR: anisotropy
        if ((0 == strncmp(token, "aniso", 5)) && IS_SPACE(token[5])) {
            token += 6;
            material.m_properties.m_anisotropy = parseReal(&token);
            continue;
        }

        // PBR: anisotropy rotation
        if ((0 == strncmp(token, "anisor", 6)) && IS_SPACE(token[6])) {
            token += 7;
            material.m_properties.m_anisotropyRotation = parseReal(&token);
            continue;
        }

        // FIXME: Commented out since incompatible with std::array member of texture data
        //// ambient texture
        //if ((0 == strncmp(token, "map_Ka", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_ambientTexture.m_textureFileName),
        //        &(material.m_ambientTexture), token);
        //    continue;
        //}

        //// diffuse texture
        //if ((0 == strncmp(token, "map_Kd", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_diffuseTexture.m_textureFileName),
        //        &(material.m_diffuseTexture), token);
        //    continue;
        //}

        //// specular texture
        //if ((0 == strncmp(token, "map_Ks", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_specularTexture.m_textureFileName),
        //        &(material.m_specularTexture), token);
        //    continue;
        //}

        //// specular highlight texture
        //if ((0 == strncmp(token, "map_Ns", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_specularHighlightTexture.m_textureFileName),
        //        &(material.m_specularHighlightTexture), token);
        //    continue;
        //}

        //// bump texture
        //if ((0 == strncmp(token, "map_bump", 8)) && IS_SPACE(token[8])) {
        //    token += 9;
        //    parseTextureNameAndOption(&(material.m_bumpTexture.m_textureFileName),
        //        &(material.m_bumpTexture), token);
        //    continue;
        //}

        //// bump texture
        //if ((0 == strncmp(token, "map_Bump", 8)) && IS_SPACE(token[8])) {
        //    token += 9;
        //    parseTextureNameAndOption(&(material.m_bumpTexture.m_textureFileName),
        //        &(material.m_bumpTexture), token);
        //    continue;
        //}

        //// bump texture
        //if ((0 == strncmp(token, "bump", 4)) && IS_SPACE(token[4])) {
        //    token += 5;
        //    parseTextureNameAndOption(&(material.m_bumpTexture.m_textureFileName),
        //        &(material.m_bumpTexture), token);
        //    continue;
        //}

        //// alpha texture
        //if ((0 == strncmp(token, "map_d", 5)) && IS_SPACE(token[5])) {
        //    token += 6;
        //    material.m_alphaTexture.m_textureFileName = token;
        //    parseTextureNameAndOption(&(material.m_alphaTexture.m_textureFileName),
        //        &(material.m_alphaTexture), token);
        //    continue;
        //}

        //// displacement texture
        //if ((0 == strncmp(token, "disp", 4)) && IS_SPACE(token[4])) {
        //    token += 5;
        //    parseTextureNameAndOption(&(material.m_displacementTexture.m_textureFileName),
        //        &(material.m_displacementTexture), token);
        //    continue;
        //}

        //// reflection map
        //if ((0 == strncmp(token, "refl", 4)) && IS_SPACE(token[4])) {
        //    token += 5;
        //    parseTextureNameAndOption(&(material.m_reflectionTexture.m_textureFileName),
        //        &(material.m_reflectionTexture), token);
        //    continue;
        //}

        //// PBR: roughness texture
        //if ((0 == strncmp(token, "map_Pr", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_roughnessTexture.m_textureFileName),
        //        &(material.m_roughnessTexture), token);
        //    continue;
        //}

        //// PBR: metallic texture
        //if ((0 == strncmp(token, "map_Pm", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_metallicTexture.m_textureFileName),
        //        &(material.m_metallicTexture), token);
        //    continue;
        //}

        //// PBR: sheen texture
        //// Rim lighting
        //if ((0 == strncmp(token, "map_Ps", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_sheenTexture.m_textureFileName),
        //        &(material.m_sheenTexture), token);
        //    continue;
        //}

        //// PBR: emissive texture
        //if ((0 == strncmp(token, "map_Ke", 6)) && IS_SPACE(token[6])) {
        //    token += 7;
        //    parseTextureNameAndOption(&(material.m_emissiveTexture.m_textureFileName),
        //        &(material.m_emissiveTexture), token);
        //    continue;
        //}

        //// PBR: normal map texture
        //if ((0 == strncmp(token, "norm", 4)) && IS_SPACE(token[4])) {
        //    token += 5;
        //    parseTextureNameAndOption(&(material.m_normalTexture.m_textureFileName),
        //        &(material.m_normalTexture), token);
        //    continue;
        //}

        // unknown parameter
        const char *_space = strchr(token, ' ');
        if (!_space) {
            _space = strchr(token, '\t');
        }
        // Commented since custom parameters were removed
        //if (_space) {
        //    std::ptrdiff_t len = _space - token;
        //    std::string key(token, static_cast<size_t>(len));
        //    std::string value = _space + 1;
        //    material.m_properties.m_customParameters.insert(
        //        std::pair<std::string, std::string>(key, value));
        //}
    }
    // flush last material.
    material_map->insert(std::pair<std::string, int>(
        (std::string)material.m_name, static_cast<int>(materials->size())));
    materials->push_back(material);

    if (warning) {
        (*warning) = warn_ss.str();
    }
}

// Loads material in loadObj routine
bool MaterialFileReader::operator()(const std::string &matId,
    std::vector<rev::MaterialData> *materials,
    std::map<std::string, int> *matMap,
    std::string *warn, std::string *err) {
    std::string filepath;

    // Get full material filepath
    if (!m_mtlBaseDir.empty()) {
        filepath = std::string(m_mtlBaseDir) + matId;
    }
    else {
        filepath = matId;
    }

    // Create input filestream
    std::ifstream matIStream(filepath.c_str());
    if (!matIStream) {
        std::stringstream ss;
        ss << "Material file [ " << filepath << " ] not found." << std::endl;
        if (warn) {
            (*warn) += ss.str();
        }
        return false;
    }

    // Load material
    loadMtl(matMap, materials, &matIStream, warn, err);

    // Set filepath for each material
    for (auto& mtl : *materials) {
        mtl.m_path = filepath.c_str();
    }

    return true;
}

bool MaterialStreamReader::operator()(const std::string &matId,
    std::vector<rev::MaterialData> *materials,
    std::map<std::string, int> *matMap,
    std::string *warn, std::string *err) {
    (void)err;
    (void)matId;
    if (!m_inStream) {
        std::stringstream ss;
        ss << "Material stream in error state. " << std::endl;
        if (warn) {
            (*warn) += ss.str();
        }
        return false;
    }

    loadMtl(matMap, materials, &m_inStream, warn, err);

    return true;
}

// Called by OBJReader
bool loadObj(rev::VertexAttributes *attrib, std::vector<shape_t> *shapes,
    std::vector<rev::MaterialData> *materials, std::string *warn,
    std::string *err, const char *filename, const char *mtl_basedir,
    bool trianglulate, bool default_vcols_fallback) {
    attrib->m_vertices.clear();
    attrib->m_normals.clear();
    attrib->m_texCoords.clear();
    attrib->m_colors.clear();
    shapes->clear();

    std::stringstream errss;

    std::ifstream ifs(filename);
    if (!ifs) {
        errss << "Cannot open file [" << filename << "]" << std::endl;
        if (err) {
            (*err) = errss.str();
        }
        return false;
    }

    std::string baseDir = mtl_basedir ? mtl_basedir : "";
    if (!baseDir.empty()) {
#ifndef _WIN32
        const char dirsep = '/';
#else
        const char dirsep = '\\';
#endif
        if (baseDir[baseDir.length() - 1] != dirsep) baseDir += dirsep;
    }
    MaterialFileReader matFileReader(baseDir);

    return loadObj(attrib, shapes, materials, warn, err, &ifs, &matFileReader,
        trianglulate, default_vcols_fallback);
}

bool loadObj(rev::VertexAttributes *attrib, std::vector<shape_t> *shapes,
    std::vector<rev::MaterialData> *materials, std::string *warn,
    std::string *err, std::istream *inStream,
    MaterialReader *readMatFn /*= NULL*/, bool triangulate,
    bool default_vcols_fallback) {
    std::stringstream errss;

    std::vector<rev::Vector3> vertices;
    std::vector<rev::Vector3> normals;
    std::vector<rev::Vector2> texCoords;
    std::vector<rev::Vector4> colors;
    std::vector<tag_t> tags;
    PrimitiveGroup prim_group;
    std::string name;

    // material
    std::map<std::string, int> material_map;
    std::vector<std::string> materialFiles;
    int material = -1;

    // smoothing group id
    unsigned int current_smoothing_id =
        0;  // Initial value. 0 means no smoothing.

    int greatest_v_idx = -1;
    int greatest_vn_idx = -1;
    int greatest_vt_idx = -1;

    shape_t shape;

    bool found_all_colors = true;

    size_t line_num = 0;
    std::string linebuf;
    while (inStream->peek() != -1) {
        safeGetline(*inStream, linebuf);

        line_num++;

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\n')
                linebuf.erase(linebuf.size() - 1);
        }
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size() - 1] == '\r')
                linebuf.erase(linebuf.size() - 1);
        }

        // Skip if empty line.
        if (linebuf.empty()) {
            continue;
        }

        // Skip leading space.
        const char *token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0') continue;  // empty line

        if (token[0] == '#') continue;  // comment line

        // vertex
        if (token[0] == 'v' && IS_SPACE((token[1]))) {
            token += 2;
            rev::Vector3 v;
            Real_t r, g, b;

            found_all_colors &= parseVertexWithColor(&v[0], &v[1], &v[2], &r, &g, &b, &token);

            vertices.push_back(v);

            if (found_all_colors || default_vcols_fallback) {
                colors.push_back({ r, g, b, 1});
            }

            continue;
        }

        // normal
        if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
            token += 3;
            rev::Vector3 n;
            parseReal3(&n[0], &n[1], &n[2], &token);
            normals.push_back(n);
            continue;
        }

        // texcoord
        if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
            token += 3;
            rev::Vector2 tc;
            parseReal2(&tc[0], &tc[1], &token);
            texCoords.push_back(tc);
            continue;
        }

        // line
        if (token[0] == 'l' && IS_SPACE((token[1]))) {
            token += 2;

            __line_t line;

            while (!IS_NEW_LINE(token[0])) {
                vertex_index_t vi;
                if (!parseTriple(&token, static_cast<int>(vertices.size()),
                    static_cast<int>(normals.size()),
                    static_cast<int>(texCoords.size()), &vi)) {
                    if (err) {
                        std::stringstream ss;
                        ss << "Failed parse `l' line(e.g. zero value for vertex index. "
                            "line "
                            << line_num << ".)\n";
                        (*err) += ss.str();
                    }
                    return false;
                }

                line.vertex_indices.push_back(vi);

                size_t n = strspn(token, " \t\r");
                token += n;
            }

            prim_group.lineGroup.push_back(line);

            continue;
        }

        // points
        if (token[0] == 'p' && IS_SPACE((token[1]))) {
            token += 2;

            __points_t pts;

            while (!IS_NEW_LINE(token[0])) {
                vertex_index_t vi;
                if (!parseTriple(&token, static_cast<int>(vertices.size() ),
                    static_cast<int>(normals.size()),
                    static_cast<int>(texCoords.size()), &vi)) {
                    if (err) {
                        std::stringstream ss;
                        ss << "Failed parse `p' line(e.g. zero value for vertex index. "
                            "line "
                            << line_num << ".)\n";
                        (*err) += ss.str();
                    }
                    return false;
                }

                pts.vertex_indices.push_back(vi);

                size_t n = strspn(token, " \t\r");
                token += n;
            }

            prim_group.pointsGroup.push_back(pts);

            continue;
        }

        // face
        if (token[0] == 'f' && IS_SPACE((token[1]))) {
            token += 2;
            token += strspn(token, " \t");

            face_t face;

            face.smoothing_group_id = current_smoothing_id;
            face.vertex_indices.reserve(3);

            while (!IS_NEW_LINE(token[0])) {
                vertex_index_t vi;
                if (!parseTriple(&token, static_cast<int>(vertices.size()),
                    static_cast<int>(normals.size()),
                    static_cast<int>(texCoords.size()), &vi)) {
                    if (err) {
                        std::stringstream ss;
                        ss << "Failed parse `f' line(e.g. zero value for face index. line "
                            << line_num << ".)\n";
                        (*err) += ss.str();
                    }
                    return false;
                }

                greatest_v_idx = greatest_v_idx > vi.m_v_idx ? greatest_v_idx : vi.m_v_idx;
                greatest_vn_idx =
                    greatest_vn_idx > vi.m_vn_idx ? greatest_vn_idx : vi.m_vn_idx;
                greatest_vt_idx =
                    greatest_vt_idx > vi.m_vt_idx ? greatest_vt_idx : vi.m_vt_idx;

                face.vertex_indices.push_back(vi);
                size_t n = strspn(token, " \t\r");
                token += n;
            }

            // replace with emplace_back + std::move on C++11
            prim_group.faceGroup.push_back(face);

            continue;
        }

        // use mtl
        if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
            token += 7;
            std::stringstream ss;
            ss << token;
            std::string namebuf = ss.str();

            int newMaterialId = -1;
            if (material_map.find(namebuf) != material_map.end()) {
                newMaterialId = material_map[namebuf];
            }
            else {
#ifdef DEBUG_MODE
                (*err) += "Error, mtl " + namebuf + " not found\n";
#endif
                // { error!! material not found }
            }

            if (newMaterialId != material) {
                // Create per-face material. Thus we don't add `shape` to `shapes` at
                // this time.
                // just clear `faceGroup` after `exportGroupsToShape()` call.
                exportGroupsToShape(&shape, prim_group, tags, material, name,
                    triangulate, vertices);
                prim_group.faceGroup.clear();
                material = newMaterialId;

                // TODO: Enforce a new shape for each material
            }

            continue;
        }

        //load material (.mtl file)
        if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
            // If string contains mtllib token
            if (readMatFn) {
                token += 7; // move past mtllib text

                std::vector<std::string> filenames;
                splitString(std::string(token), ' ', filenames);

                if (filenames.empty()) {
                    if (warn) {
                        std::stringstream ss;
                        ss << "Looks like empty filename for mtllib. Use default "
                            "material (line "
                            << line_num << ".)\n";

                        (*warn) += ss.str();
                    }
                }
                else {
                    bool found = false;
                    for (size_t s = 0; s < filenames.size(); s++) {
                        if (std::find(materialFiles.begin(), materialFiles.end(), filenames.at(s)) 
                            != materialFiles.end()) {
                            // Skip if .mtl file already loaded
                            continue;
                        }
                        // Add to list of filenames
                        materialFiles.push_back(filenames[s]);
                        std::string warn_mtl;
                        std::string err_mtl;
                        bool ok = (*readMatFn)(filenames[s].c_str(), materials,
                            &material_map, &warn_mtl, &err_mtl);
                        if (warn && (!warn_mtl.empty())) {
                            (*warn) += warn_mtl;
                        }

                        if (err && (!err_mtl.empty())) {
                            (*err) += err_mtl;
                        }

                        if (ok) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        if (warn) {
                            (*warn) +=
                                "Failed to load material file(s). Use default "
                                "material.\n";
                        }
                    }
                }
            }

            continue;
        }

        // group name
        if (token[0] == 'g' && IS_SPACE((token[1]))) {
            // flush previous face group.
            bool ret = exportGroupsToShape(&shape, prim_group, tags, material, name,
                triangulate, vertices);
            (void)ret;  // return value not used.

            if (shape.m_mesh.m_indices.size() > 0) {
                shapes->push_back(shape);
            }

            shape = shape_t();

            // material = -1;
            prim_group.clear();

            std::vector<std::string> names;

            while (!IS_NEW_LINE(token[0])) {
                std::string str = parseString(&token).c_str();
                names.push_back(str);
                token += strspn(token, " \t\r");  // skip tag
            }

            // names[0] must be 'g'

            if (names.size() < 2) {
                // 'g' with empty names
                if (warn) {
                    std::stringstream ss;
                    ss << "Empty group m_name. line: " << line_num << "\n";
                    (*warn) += ss.str();
                    name = "";
                }
            }
            else {
                std::stringstream ss;
                ss << names[1];

                // tinyobjloader does not support multiple groups for a primitive.
                // Currently we concatinate multiple group names with a space to get
                // single group name.

                for (size_t i = 2; i < names.size(); i++) {
                    ss << " " << names[i];
                }

                name = ss.str();
            }

            continue;
        }

        // object name
        if (token[0] == 'o' && IS_SPACE((token[1]))) {
            // flush previous face group.
            bool ret = exportGroupsToShape(&shape, prim_group, tags, material, name,
                triangulate, vertices);
            if (ret) {
                shapes->push_back(shape);
            }

            // material = -1;
            prim_group.clear();
            shape = shape_t();

            // @todo { multiple object name? }
            token += 2;
            std::stringstream ss;
            ss << token;
            name = ss.str();

            continue;
        }

        if (token[0] == 't' && IS_SPACE(token[1])) {
            const int max_tag_nums = 8192;  // FIXME(syoyo): Parameterize.
            tag_t tag;

            token += 2;

            tag.name = (const char*)parseString(&token);

            tag_sizes ts = parseTagTriple(&token);

            if (ts.num_ints < 0) {
                ts.num_ints = 0;
            }
            if (ts.num_ints > max_tag_nums) {
                ts.num_ints = max_tag_nums;
            }

            if (ts.num_reals < 0) {
                ts.num_reals = 0;
            }
            if (ts.num_reals > max_tag_nums) {
                ts.num_reals = max_tag_nums;
            }

            if (ts.num_strings < 0) {
                ts.num_strings = 0;
            }
            if (ts.num_strings > max_tag_nums) {
                ts.num_strings = max_tag_nums;
            }

            tag.intValues.resize(static_cast<size_t>(ts.num_ints));

            for (size_t i = 0; i < static_cast<size_t>(ts.num_ints); ++i) {
                tag.intValues[i] = parseInt(&token);
            }

            tag.floatValues.resize(static_cast<size_t>(ts.num_reals));
            for (size_t i = 0; i < static_cast<size_t>(ts.num_reals); ++i) {
                tag.floatValues[i] = parseReal(&token);
            }

            tag.stringValues.resize(static_cast<size_t>(ts.num_strings));
            for (size_t i = 0; i < static_cast<size_t>(ts.num_strings); ++i) {
                tag.stringValues[i] = (const char*)parseString(&token);
            }

            tags.push_back(tag);

            continue;
        }

        if (token[0] == 's' && IS_SPACE(token[1])) {
            // smoothing group id
            token += 2;

            // skip space.
            token += strspn(token, " \t");  // skip space

            if (token[0] == '\0') {
                continue;
            }

            if (token[0] == '\r' || token[1] == '\n') {
                continue;
            }

            if (strlen(token) >= 3) {
                if (token[0] == 'o' && token[1] == 'f' && token[2] == 'f') {
                    current_smoothing_id = 0;
                }
            }
            else {
                // assume number
                int smGroupId = parseInt(&token);
                if (smGroupId < 0) {
                    // parse error. force set to 0.
                    // FIXME(syoyo): Report warning.
                    current_smoothing_id = 0;
                }
                else {
                    current_smoothing_id = static_cast<unsigned int>(smGroupId);
                }
            }

            continue;
        }  // smoothing group id

        // Ignore unknown command.
    }

    // not all vertices have colors, no default colors desired? -> clear colors
    if (!found_all_colors && !default_vcols_fallback) {
        colors.clear();
    }

    if (greatest_v_idx >= static_cast<int>(vertices.size())) {
        if (warn) {
            std::stringstream ss;
            ss << "Vertex m_indices out of bounds (line " << line_num << ".)\n"
                << std::endl;
            (*warn) += ss.str();
        }
    }
    if (greatest_vn_idx >= static_cast<int>(normals.size())) {
        if (warn) {
            std::stringstream ss;
            ss << "Vertex normal m_indices out of bounds (line " << line_num << ".)\n"
                << std::endl;
            (*warn) += ss.str();
        }
    }
    if (greatest_vt_idx >= static_cast<int>(texCoords.size())) {
        if (warn) {
            std::stringstream ss;
            ss << "Vertex texcoord m_indices out of bounds (line " << line_num << ".)\n"
                << std::endl;
            (*warn) += ss.str();
        }
    }

    bool ret = exportGroupsToShape(&shape, prim_group, tags, material, name,
        triangulate, vertices);
    // exportGroupsToShape return false when `usemtl` is called in the last
    // line.
    // we also add `shape` to `shapes` when `shape.vertexData` has already some
    // faces(indices)
    if (ret || shape.m_mesh.m_indices
        .size()) {  // FIXME(syoyo): Support other prims(e.g. lines)
        shapes->push_back(shape);
    }
    prim_group.clear();  // for safety

    if (err) {
        (*err) += errss.str();
    }

    attrib->m_vertices.swap(vertices);
    attrib->m_normals.swap(normals);
    attrib->m_texCoords.swap(texCoords);
    attrib->m_colors.swap(colors);

    return true;
}


#ifdef __clang__
#pragma clang diagnostic pop
#endif
}  // namespace tinyobj

#endif
