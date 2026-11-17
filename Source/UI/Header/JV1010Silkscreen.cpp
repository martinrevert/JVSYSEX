#include "JV1010Silkscreen.h"
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace jv1010 {
namespace ui {

namespace {

using Poly = std::vector<ImVec2>;

float signedArea(const Poly& p) {
    double a = 0.0;
    const size_t n = p.size();
    for (size_t i = 0; i < n; ++i) {
        const ImVec2& p0 = p[i];
        const ImVec2& p1 = p[(i + 1) % n];
        a += static_cast<double>(p0.x) * p1.y - static_cast<double>(p1.x) * p0.y;
    }
    return static_cast<float>(a * 0.5);
}

void removeConsecutiveDuplicates(Poly& p, float eps = 1e-4f) {
    if (p.empty()) return;
    Poly out;
    out.reserve(p.size());
    for (const auto& pt : p) {
        if (out.empty() || std::fabs(out.back().x - pt.x) > eps || std::fabs(out.back().y - pt.y) > eps) {
            out.push_back(pt);
        }
    }
    if (out.size() > 1 &&
        std::fabs(out.front().x - out.back().x) <= eps &&
        std::fabs(out.front().y - out.back().y) <= eps) {
        out.pop_back();
    }
    p.swap(out);
}

// Bridges one clockwise hole into a CCW outer ring using Eberly's visible-vertex method.
void bridgeHoleIntoOuter(Poly& outer, const Poly& hole) {
    if (outer.size() < 3 || hole.size() < 3) return;

    // Find rightmost vertex M in hole
    size_t hIdx = 0;
    for (size_t i = 1; i < hole.size(); ++i) {
        if (hole[i].x > hole[hIdx].x) hIdx = i;
    }
    const ImVec2 M = hole[hIdx];

    // Ray cast +X from M against outer edges to find closest intersection I
    float bestX = 1e30f;
    size_t edgeI = 0;
    bool foundEdge = false;
    for (size_t i = 0; i < outer.size(); ++i) {
        const ImVec2& a = outer[i];
        const ImVec2& b = outer[(i + 1) % outer.size()];
        if ((a.y <= M.y && b.y > M.y) || (b.y <= M.y && a.y > M.y)) {
            float t = (M.y - a.y) / (b.y - a.y);
            float ix = a.x + t * (b.x - a.x);
            if (ix >= M.x - 1e-5f && ix < bestX) {
                bestX = ix;
                edgeI = i;
                foundEdge = true;
            }
        }
    }

    size_t oIdx = 0;
    if (foundEdge) {
        const ImVec2 I(bestX, M.y);
        const ImVec2& a = outer[edgeI];
        const ImVec2& b = outer[(edgeI + 1) % outer.size()];
        size_t pIdx = (a.x >= b.x) ? edgeI : (edgeI + 1) % outer.size();
        const ImVec2 P = outer[pIdx];

        // Check if any outer vertex lies strictly inside triangle (M, I, P)
        auto signCross = [](const ImVec2& u, const ImVec2& v, const ImVec2& w) {
            return (v.x - u.x) * (w.y - u.y) - (v.y - u.y) * (w.x - u.x);
        };
        float triArea = signCross(M, I, P);
        oIdx = pIdx;
        float bestCos = -2.0f;
        for (size_t i = 0; i < outer.size(); ++i) {
            if (i == pIdx) continue;
            const ImVec2& R = outer[i];
            if (R.x <= M.x) continue;
            float c1 = signCross(M, I, R) * (triArea >= 0.0f ? 1.0f : -1.0f);
            float c2 = signCross(I, P, R) * (triArea >= 0.0f ? 1.0f : -1.0f);
            float c3 = signCross(P, M, R) * (triArea >= 0.0f ? 1.0f : -1.0f);
            if (c1 >= 0.0f && c2 >= 0.0f && c3 >= 0.0f) {
                float dx = R.x - M.x;
                float dy = R.y - M.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 1e-5f) {
                    float cosAng = dx / len;
                    if (cosAng > bestCos) {
                        bestCos = cosAng;
                        oIdx = i;
                    }
                }
            }
        }
    } else {
        float bestD2 = 1e30f;
        for (size_t i = 0; i < outer.size(); ++i) {
            float dx = outer[i].x - M.x;
            float dy = outer[i].y - M.y;
            float d2 = dx * dx + dy * dy;
            if (d2 < bestD2) {
                bestD2 = d2;
                oIdx = i;
            }
        }
    }

    Poly merged;
    merged.reserve(outer.size() + hole.size() + 2);
    for (size_t i = 0; i <= oIdx; ++i) merged.push_back(outer[i]);
    for (size_t k = 0; k < hole.size(); ++k) {
        merged.push_back(hole[(hIdx + k) % hole.size()]);
    }
    merged.push_back(hole[hIdx]);
    merged.push_back(outer[oIdx]);
    for (size_t i = oIdx + 1; i < outer.size(); ++i) merged.push_back(outer[i]);
    outer.swap(merged);
}

bool samePt(const ImVec2& u, const ImVec2& v, float eps = 1e-4f) {
    return std::fabs(u.x - v.x) <= eps && std::fabs(u.y - v.y) <= eps;
}

bool pointInTriangle(const ImVec2& p, const ImVec2& a, const ImVec2& b, const ImVec2& c) {
    auto cross = [](const ImVec2& u, const ImVec2& v, const ImVec2& w) {
        return (v.x - u.x) * (w.y - u.y) - (v.y - u.y) * (w.x - u.x);
    };
    float c1 = cross(a, b, p);
    float c2 = cross(b, c, p);
    float c3 = cross(c, a, p);
    return (c1 >= -1e-5f && c2 >= -1e-5f && c3 >= -1e-5f);
}

// Ear-clipping triangulator for a simple CCW polygon.
void triangulateAndFill(ImDrawList* dl, Poly poly, ImU32 col) {
    removeConsecutiveDuplicates(poly);
    if (poly.size() < 3) return;
    if (signedArea(poly) < 0.0f) std::reverse(poly.begin(), poly.end());

    std::vector<size_t> idx(poly.size());
    for (size_t i = 0; i < poly.size(); ++i) idx[i] = i;

    size_t guard = 0;
    const size_t maxGuard = poly.size() * poly.size() + 16;
    while (idx.size() > 3 && guard++ < maxGuard) {
        bool clipped = false;
        const size_t n = idx.size();
        for (size_t i = 0; i < n; ++i) {
            size_t iPrev = idx[(i + n - 1) % n];
            size_t iCurr = idx[i];
            size_t iNext = idx[(i + 1) % n];
            const ImVec2& a = poly[iPrev];
            const ImVec2& b = poly[iCurr];
            const ImVec2& c = poly[iNext];

            float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross <= 1e-6f) continue;

            bool containsOther = false;
            for (size_t j = 0; j < n; ++j) {
                size_t ij = idx[j];
                if (ij == iPrev || ij == iCurr || ij == iNext) continue;
                const ImVec2& pt = poly[ij];
                // Skip duplicate vertices introduced by hole-bridging
                if (samePt(pt, a) || samePt(pt, b) || samePt(pt, c)) continue;
                if (pointInTriangle(pt, a, b, c)) {
                    containsOther = true;
                    break;
                }
            }
            if (containsOther) continue;

            dl->AddTriangleFilled(a, b, c, col);
            idx.erase(idx.begin() + static_cast<ptrdiff_t>(i));
            clipped = true;
            break;
        }
        if (!clipped) break;
    }
    if (idx.size() == 3) {
        dl->AddTriangleFilled(poly[idx[0]], poly[idx[1]], poly[idx[2]], col);
    }
}

void appendCubicBezier(Poly& dst, ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, int steps = 10) {
    for (int i = 1; i <= steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float u = 1.0f - t;
        float b0 = u * u * u;
        float b1 = 3.0f * u * u * t;
        float b2 = 3.0f * u * t * t;
        float b3 = t * t * t;
        dst.push_back(ImVec2(b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
                             b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y));
    }
}

// Parses SVG path string (supports M/m, L/l, H/h, V/v, C/c, S/s, Z/z) into subpaths.
std::vector<Poly> parseSvgPath(const char* d, float ox, float oy, float sx, float sy) {
    std::vector<Poly> paths;
    Poly cur;
    ImVec2 pos(0.0f, 0.0f);
    ImVec2 start(0.0f, 0.0f);
    ImVec2 lastCtrl(0.0f, 0.0f);
    char prevCmd = 0;
    char cmd = 0;

    auto xf = [&](ImVec2 p) { return ImVec2(ox + p.x * sx, oy + p.y * sy); };

    const char* p = d;
    auto skipSep = [&]() {
        while (*p && (*p == ' ' || *p == ',' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
    };
    auto hasNum = [&]() {
        skipSep();
        return *p && (std::isdigit(static_cast<unsigned char>(*p)) || *p == '-' || *p == '+' || *p == '.');
    };
    auto nextFloat = [&]() -> float {
        skipSep();
        char* endPtr = nullptr;
        float v = std::strtof(p, &endPtr);
        p = endPtr;
        return v;
    };

    while (*p) {
        skipSep();
        if (!*p) break;
        if (std::isalpha(static_cast<unsigned char>(*p))) {
            cmd = *p++;
        } else if (!hasNum()) {
            ++p;
            continue;
        }

        if (cmd == 'Z' || cmd == 'z') {
            if (!cur.empty()) {
                paths.push_back(std::move(cur));
                cur.clear();
            }
            pos = start;
            prevCmd = cmd;
            cmd = 0;
            continue;
        }

        if (cmd == 'M' || cmd == 'm') {
            if (!cur.empty()) {
                paths.push_back(std::move(cur));
                cur.clear();
            }
            float x = nextFloat();
            float y = nextFloat();
            pos = (cmd == 'm') ? ImVec2(pos.x + x, pos.y + y) : ImVec2(x, y);
            start = pos;
            cur.push_back(xf(pos));
            prevCmd = cmd;
            cmd = (cmd == 'm') ? 'l' : 'L';
            continue;
        }

        if (cmd == 'L' || cmd == 'l') {
            float x = nextFloat();
            float y = nextFloat();
            pos = (cmd == 'l') ? ImVec2(pos.x + x, pos.y + y) : ImVec2(x, y);
            cur.push_back(xf(pos));
            prevCmd = cmd;
        } else if (cmd == 'H' || cmd == 'h') {
            float x = nextFloat();
            pos.x = (cmd == 'h') ? (pos.x + x) : x;
            cur.push_back(xf(pos));
            prevCmd = cmd;
        } else if (cmd == 'V' || cmd == 'v') {
            float y = nextFloat();
            pos.y = (cmd == 'v') ? (pos.y + y) : y;
            cur.push_back(xf(pos));
            prevCmd = cmd;
        } else if (cmd == 'C' || cmd == 'c') {
            float x1 = nextFloat(), y1 = nextFloat();
            float x2 = nextFloat(), y2 = nextFloat();
            float x = nextFloat(), y = nextFloat();
            ImVec2 c1 = (cmd == 'c') ? ImVec2(pos.x + x1, pos.y + y1) : ImVec2(x1, y1);
            ImVec2 c2 = (cmd == 'c') ? ImVec2(pos.x + x2, pos.y + y2) : ImVec2(x2, y2);
            ImVec2 end = (cmd == 'c') ? ImVec2(pos.x + x, pos.y + y) : ImVec2(x, y);
            appendCubicBezier(cur, xf(pos), xf(c1), xf(c2), xf(end), 10);
            lastCtrl = c2;
            pos = end;
            prevCmd = cmd;
        } else if (cmd == 'S' || cmd == 's') {
            ImVec2 c1 = (prevCmd == 'C' || prevCmd == 'c' || prevCmd == 'S' || prevCmd == 's')
                            ? ImVec2(2.0f * pos.x - lastCtrl.x, 2.0f * pos.y - lastCtrl.y)
                            : pos;
            float x2 = nextFloat(), y2 = nextFloat();
            float x = nextFloat(), y = nextFloat();
            ImVec2 c2 = (cmd == 's') ? ImVec2(pos.x + x2, pos.y + y2) : ImVec2(x2, y2);
            ImVec2 end = (cmd == 's') ? ImVec2(pos.x + x, pos.y + y) : ImVec2(x, y);
            appendCubicBezier(cur, xf(pos), xf(c1), xf(c2), xf(end), 10);
            lastCtrl = c2;
            pos = end;
            prevCmd = cmd;
        }
    }
    if (!cur.empty()) paths.push_back(std::move(cur));
    return paths;
}

// Renders a single glyph SVG path (with 0 or 1 hole) as anti-aliased solid triangles.
void drawGlyphSvg(ImDrawList* dl, const char* svgPath, bool hasHole,
                  float ox, float oy, float scaleX, float scaleY, ImU32 col) {
    auto subpaths = parseSvgPath(svgPath, ox, oy, scaleX, scaleY);
    if (subpaths.empty()) return;

    for (auto& sp : subpaths) removeConsecutiveDuplicates(sp);

    if (hasHole && subpaths.size() >= 2) {
        Poly& outer = subpaths[0];
        if (signedArea(outer) < 0.0f) std::reverse(outer.begin(), outer.end());
        for (size_t i = 1; i < subpaths.size(); ++i) {
            Poly hole = subpaths[i];
            if (signedArea(hole) > 0.0f) std::reverse(hole.begin(), hole.end());
            dl->AddPolyline(hole.data(), static_cast<int>(hole.size()), col, ImDrawFlags_Closed, 0.75f);
            bridgeHoleIntoOuter(outer, hole);
        }
        dl->AddPolyline(subpaths[0].data(), static_cast<int>(subpaths[0].size()), col, ImDrawFlags_Closed, 0.75f);
        triangulateAndFill(dl, outer, col);
    } else {
        for (auto& sp : subpaths) {
            if (sp.size() < 3) continue;
            dl->AddPolyline(sp.data(), static_cast<int>(sp.size()), col, ImDrawFlags_Closed, 0.75f);
            triangulateAndFill(dl, sp, col);
        }
    }
}

// Official Roland Corporation wordmark SVG paths (normalized from static.roland.com/global/css/fonts/roland.svg)
// Original bounding box: x in [33.89 .. 121.00] (w=87.11), y in [0.73 .. 19.68] (h=18.95)
void drawOfficialRolandLogo(ImDrawList* dl, float ox, float oy, float capH, ImU32 col, float& outWidth) {
    const float scale = capH / 18.95f;
    const float tx = ox - 33.89f * scale;
    const float ty = oy - 0.73f * scale;

    // 'R' (outer + counter hole)
    drawGlyphSvg(dl,
        "M50.5,19.19c.13,0,.24-.11.24-.24,0-.04,0-.06-.03-.12-.83-1.72-1.19-4.55-1.54-6.06-.35-1.49-.66-2.1-1.15-2.49-.19-.15-.13-.28.01-.38,1.33-.89,2.29-2.48,2.29-4.11,0-3.46-2.46-5.07-5.64-5.07h-10.3c-.5,0-.74.29-.74.74v16.98c0,.45.24.74.74.74h1.68c.5,0,.74-.29.74-.74v-6.82c0-.45.24-.74.73-.74h5.37c.52,0,1.34-.02,1.95.45.48.38.81.92,1.02,1.49.21.57.32,1.18.42,1.78.17.99.33,1.98.51,2.96.25,1.33.66,1.61,1.44,1.61h2.26Z"
        "M43.89,3.16c1.62,0,3.07.73,3.07,2.64,0,2.23-1.85,2.64-3.07,2.65h-6.36c-.5,0-.74-.29-.74-.74v-3.81c0-.45.24-.74.74-.74h6.36Z",
        true, tx, ty, scale, scale, col);

    // 'o' (outer + counter hole)
    drawGlyphSvg(dl,
        "M59.36,5.54c5.7,0,8.39,3.71,8.39,7.07s-2.69,7.07-8.39,7.07c-5.7,0-8.39-3.71-8.39-7.07,0-3.36,2.69-7.07,8.39-7.07Z"
        "M59.36,17.13c3.34,0,4.97-2.32,4.97-4.53s-1.63-4.53-4.97-4.53c-3.34,0-4.97,2.32-4.97,4.53,0,2.21,1.63,4.53,4.97,4.53Z",
        true, tx, ty, scale, scale, col);

    // 'l'
    drawGlyphSvg(dl,
        "M72.39,1.47c0-.45-.24-.74-.74-.74h-2.01c-.5,0-.74.29-.74.74v16.98c0,.45.24.74.74.74h2.01c.5,0,.74-.29.74-.74V1.47Z",
        false, tx, ty, scale, scale, col);

    // 'a' (outer + counter hole)
    drawGlyphSvg(dl,
        "M87.75,10.81c0-4.18-3.63-5.28-6.88-5.28-1.83,0-3.43.43-4.62,1.32-1.39,1.04-1.95,2.56-1.95,2.56-.08.28.21.55.47.55h2.29c.49,0,.63-.07.89-.51.58-.98,1.54-1.43,3.31-1.43,1.98,0,2.82.66,3.16,1.43.34.77.18,1.17-.46,1.21-4.78.24-6.73.48-7.81.99s-2.4,1.75-2.38,3.63c0,1.26.57,2.51,1.92,3.45,2.27,1.61,6.5,1.21,8.46-.48.24-.2.51-.04.58.21.19.59.46.73,1.05.74h1.22c.5,0,.74-.29.74-.74v-7.64Z"
        "M77.76,16.42c-.39-.26-.65-.62-.65-1.15,0-.66.21-1.1.75-1.4,1.64-.93,5.55-.84,6.16-.71.2.04.47.17.47.51,0,3.34-4.72,4.1-6.74,2.76Z",
        true, tx, ty, scale, scale, col);

    // 'n'
    drawGlyphSvg(dl,
        "M92.26,19.19c.5,0,.74-.29.74-.74v-5.05c0-.94.03-1.75.29-2.44.69-1.88,2.37-2.83,4.42-2.83,2.29,0,3.01,1.29,3.01,2.88v7.43c0,.45.24.74.74.74h1.74c.5,0,.74-.29.74-.74v-7.77c0-3.63-2.98-5.14-5.55-5.14-2.21,0-4.05,1.03-4.9,1.69-.19.15-.48.23-.48-.08v-.78c0-.45-.24-.74-.74-.74h-1.76c-.5,0-.74.29-.74.74v12.09c0,.45.24.74.74.74h1.76Z",
        false, tx, ty, scale, scale, col);

    // 'd' (outer + counter hole)
    drawGlyphSvg(dl,
        "M121,1.47c0-.45-.24-.74-.74-.74h-1.74c-.5,0-.74.29-.74.74v5.33c0,.4-.21.51-.42.37-4.77-3.63-12,0-11.99,5.39.01,6.06,7.14,9.29,11.98,5.48.21-.17.43-.04.42.34,0,.06,0,.8.74.8h1.73c.5,0,.74-.29.74-.74V1.47Z"
        "M117.71,12.61c0,2.75-2.06,4.58-4.57,4.58s-4.43-2.02-4.43-4.62c0-2.58,2.08-4.36,4.43-4.36,2.51,0,4.57,1.71,4.57,4.4Z",
        true, tx, ty, scale, scale, col);

    outWidth = 87.11f * scale;
}

// Draws "JV-1010" using crisp filled geometric paths matching the hardware silkscreen.
void drawJV1010ModelFilled(ImDrawList* dl, float ox, float oy, float capH, ImU32 col, float& outWidth) {
    const float s = capH;
    float x = ox;

    // 'J': flat top, straight right stem, rounded bottom-right, 45-deg chamfered left tail
    drawGlyphSvg(dl,
        "M0.58,0.02 H0.74 V0.74 C0.74,0.92 0.63,0.98 0.45,0.98 H0.02 L0.13,0.83 H0.44 C0.54,0.83 0.58,0.79 0.58,0.69 Z",
        false, x, oy, s, s, col);
    x += 0.80f * s;

    // 'V': flat-topped wide diagonal arms meeting at flat bottom
    drawGlyphSvg(dl,
        "M0.00,0.02 H0.18 L0.42,0.81 L0.66,0.02 H0.84 L0.51,0.98 H0.33 Z",
        false, x, oy, s, s, col);
    x += 0.88f * s;

    // '-'
    drawGlyphSvg(dl,
        "M0.02,0.46 H0.32 V0.59 H0.02 Z",
        false, x, oy, s, s, col);
    x += 0.37f * s;

    // '1': sharp angled top-left flag + vertical stem
    drawGlyphSvg(dl,
        "M0.02,0.19 L0.16,0.02 H0.30 V0.98 H0.14 V0.20 L0.02,0.29 Z",
        false, x, oy, s, s, col);
    x += 0.36f * s;

    // '0': wide Eurostile/Microgramma squircle (outer + inner hole)
    const char* zeroSvg =
        "M0.24,0.02 H0.56 C0.74,0.02 0.80,0.09 0.80,0.26 V0.74 C0.80,0.91 0.74,0.98 0.56,0.98 H0.24 C0.06,0.98 0.00,0.91 0.00,0.74 V0.26 C0.00,0.09 0.06,0.02 0.24,0.02 Z"
        "M0.26,0.16 C0.17,0.16 0.15,0.19 0.15,0.28 V0.72 C0.15,0.81 0.17,0.84 0.26,0.84 H0.54 C0.63,0.84 0.65,0.81 0.65,0.72 V0.28 C0.65,0.19 0.63,0.16 0.54,0.16 Z";

    drawGlyphSvg(dl, zeroSvg, true, x, oy, s, s, col);
    x += 0.86f * s;

    // '1'
    drawGlyphSvg(dl,
        "M0.02,0.19 L0.16,0.02 H0.30 V0.98 H0.14 V0.20 L0.02,0.29 Z",
        false, x, oy, s, s, col);
    x += 0.36f * s;

    // '0'
    drawGlyphSvg(dl, zeroSvg, true, x, oy, s, s, col);
    x += 0.80f * s;

    outWidth = x - ox;
}

struct VecCanvas {
    ImDrawList* dl;
    float ox;
    float oy;
    float scale;
    float slant{ 0.0f };

    ImVec2 pt(float x, float y) const {
        float sx = x + slant * (1.0f - y);
        return ImVec2(ox + sx * scale, oy + y * scale);
    }

    void line(float x0, float y0, float x1, float y1, ImU32 col, float thick) const {
        dl->AddLine(pt(x0, y0), pt(x1, y1), col, thick);
    }

    void beginPath() const { dl->PathClear(); }
    void moveTo(float x, float y) const { dl->PathLineTo(pt(x, y)); }
    void lineTo(float x, float y) const { dl->PathLineTo(pt(x, y)); }
    void cubicTo(float cx1, float cy1, float cx2, float cy2, float x, float y, int segs = 10) const {
        dl->PathBezierCubicCurveTo(pt(cx1, cy1), pt(cx2, cy2), pt(x, y), segs);
    }
    void strokePath(ImU32 col, float thick, bool closed = false) const {
        dl->PathStroke(col, closed ? ImDrawFlags_Closed : ImDrawFlags_None, thick);
    }
};

float drawVectorChar(ImDrawList* dl, float ox, float oy, float charH, float slant,
                     char ch, ImU32 col, float thick, float widthScale) {
    VecCanvas c{ dl, ox, oy, charH, slant };
    const float w = 0.72f * widthScale;

    switch (ch) {
    case ' ': return 0.34f * widthScale;
    case '"':
        c.line(0.10f, 0.04f, 0.10f, 0.26f, col, thick);
        c.line(0.26f, 0.04f, 0.26f, 0.26f, col, thick);
        return 0.36f * widthScale;
    case '+':
        c.line(0.08f, 0.56f, 0.48f, 0.56f, col, thick);
        c.line(0.28f, 0.34f, 0.28f, 0.78f, col, thick);
        return 0.56f * widthScale;

    case '6':
        c.beginPath();
        c.moveTo(w * 0.88f, 0.14f);
        c.cubicTo(w * 0.84f, 0.03f, w * 0.12f, 0.03f, w * 0.08f, 0.26f);
        c.lineTo(w * 0.08f, 0.78f);
        c.cubicTo(w * 0.08f, 0.96f, w * 0.90f, 0.96f, w * 0.90f, 0.74f);
        c.lineTo(w * 0.90f, 0.60f);
        c.cubicTo(w * 0.90f, 0.46f, w * 0.10f, 0.46f, w * 0.08f, 0.58f);
        c.strokePath(col, thick, false);
        return w + 0.14f;
    case '4':
        c.beginPath();
        c.moveTo(w * 0.76f, 0.95f);
        c.lineTo(w * 0.76f, 0.06f);
        c.lineTo(w * 0.06f, 0.68f);
        c.lineTo(w * 0.92f, 0.68f);
        c.strokePath(col, thick, false);
        return w + 0.14f;

    case 'A':
        c.beginPath();
        c.moveTo(0.05f, 0.95f);
        c.lineTo(w * 0.50f, 0.06f);
        c.lineTo(w - 0.05f, 0.95f);
        c.strokePath(col, thick, false);
        c.line(w * 0.20f, 0.66f, w * 0.80f, 0.66f, col, thick);
        return w + 0.13f;
    case 'B':
        c.line(0.08f, 0.06f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(w * 0.68f, 0.06f);
        c.cubicTo(w * 0.94f, 0.06f, w * 0.94f, 0.49f, w * 0.68f, 0.49f);
        c.lineTo(0.08f, 0.49f);
        c.strokePath(col, thick, false);
        c.beginPath();
        c.moveTo(0.08f, 0.49f);
        c.lineTo(w * 0.70f, 0.49f);
        c.cubicTo(w * 0.96f, 0.49f, w * 0.96f, 0.95f, w * 0.70f, 0.95f);
        c.lineTo(0.08f, 0.95f);
        c.strokePath(col, thick, false);
        return w + 0.14f;
    case 'C':
        c.beginPath();
        c.moveTo(w * 0.90f, 0.24f);
        c.cubicTo(w * 0.86f, 0.04f, w * 0.08f, 0.04f, 0.08f, 0.28f);
        c.lineTo(0.08f, 0.73f);
        c.cubicTo(0.08f, 0.97f, w * 0.86f, 0.97f, w * 0.90f, 0.77f);
        c.strokePath(col, thick, false);
        return w + 0.14f;
    case 'D':
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(w * 0.58f, 0.06f);
        c.cubicTo(w * 0.94f, 0.06f, w * 0.94f, 0.95f, w * 0.58f, 0.95f);
        c.lineTo(0.08f, 0.95f);
        c.strokePath(col, thick, true);
        return w + 0.14f;
    case 'E':
        c.beginPath();
        c.moveTo(w * 0.88f, 0.06f);
        c.lineTo(0.08f, 0.06f);
        c.lineTo(0.08f, 0.95f);
        c.lineTo(w * 0.88f, 0.95f);
        c.strokePath(col, thick, false);
        c.line(0.08f, 0.50f, w * 0.74f, 0.50f, col, thick);
        return w + 0.13f;
    case 'H':
        c.line(0.08f, 0.06f, 0.08f, 0.95f, col, thick);
        c.line(w * 0.88f, 0.06f, w * 0.88f, 0.95f, col, thick);
        c.line(0.08f, 0.50f, w * 0.88f, 0.50f, col, thick);
        return w + 0.14f;
    case 'I':
        c.line(0.14f, 0.06f, 0.14f, 0.95f, col, thick);
        return 0.28f + 0.12f;
    case 'L':
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(0.08f, 0.95f);
        c.lineTo(w * 0.86f, 0.95f);
        c.strokePath(col, thick, false);
        return w + 0.12f;
    case 'M':
        c.beginPath();
        c.moveTo(0.08f, 0.95f);
        c.lineTo(0.08f, 0.06f);
        c.lineTo(w * 0.52f, 0.68f);
        c.lineTo(w * 0.96f, 0.06f);
        c.lineTo(w * 0.96f, 0.95f);
        c.strokePath(col, thick, false);
        return w * 1.04f + 0.14f;
    case 'N':
        c.beginPath();
        c.moveTo(0.08f, 0.95f);
        c.lineTo(0.08f, 0.06f);
        c.lineTo(w * 0.88f, 0.95f);
        c.lineTo(w * 0.88f, 0.06f);
        c.strokePath(col, thick, false);
        return w + 0.14f;
    case 'O':
        c.beginPath();
        c.moveTo(w * 0.49f, 0.06f);
        c.cubicTo(w * 0.94f, 0.06f, w * 0.94f, 0.95f, w * 0.49f, 0.95f);
        c.cubicTo(0.04f, 0.95f, 0.04f, 0.06f, w * 0.49f, 0.06f);
        c.strokePath(col, thick, true);
        return w + 0.14f;
    case 'P':
        c.line(0.08f, 0.06f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(w * 0.68f, 0.06f);
        c.cubicTo(w * 0.94f, 0.06f, w * 0.94f, 0.52f, w * 0.68f, 0.52f);
        c.lineTo(0.08f, 0.52f);
        c.strokePath(col, thick, false);
        return w + 0.13f;
    case 'R':
        c.line(0.08f, 0.06f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(w * 0.68f, 0.06f);
        c.cubicTo(w * 0.94f, 0.06f, w * 0.94f, 0.50f, w * 0.68f, 0.50f);
        c.lineTo(0.08f, 0.50f);
        c.strokePath(col, thick, false);
        c.line(w * 0.54f, 0.50f, w * 0.90f, 0.95f, col, thick);
        return w + 0.14f;
    case 'S':
        c.beginPath();
        c.moveTo(w * 0.88f, 0.20f);
        c.cubicTo(w * 0.84f, 0.04f, 0.08f, 0.04f, 0.08f, 0.26f);
        c.cubicTo(0.08f, 0.49f, w * 0.90f, 0.49f, w * 0.90f, 0.73f);
        c.cubicTo(w * 0.90f, 0.96f, 0.08f, 0.96f, 0.06f, 0.78f);
        c.strokePath(col, thick, false);
        return w + 0.13f;
    case 'T':
        c.line(0.04f, 0.06f, w * 0.92f, 0.06f, col, thick);
        c.line(w * 0.48f, 0.06f, w * 0.48f, 0.95f, col, thick);
        return w + 0.12f;
    case 'U':
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(0.08f, 0.72f);
        c.cubicTo(0.08f, 0.96f, w * 0.88f, 0.96f, w * 0.88f, 0.72f);
        c.lineTo(w * 0.88f, 0.06f);
        c.strokePath(col, thick, false);
        return w + 0.14f;
    case 'V':
        c.beginPath();
        c.moveTo(0.06f, 0.06f);
        c.lineTo(w * 0.49f, 0.95f);
        c.lineTo(w * 0.92f, 0.06f);
        c.strokePath(col, thick, false);
        return w + 0.13f;
    case 'X':
        c.line(0.08f, 0.06f, w * 0.88f, 0.95f, col, thick);
        c.line(w * 0.88f, 0.06f, 0.08f, 0.95f, col, thick);
        return w + 0.13f;
    case 'Y':
        c.beginPath();
        c.moveTo(0.06f, 0.06f);
        c.lineTo(w * 0.48f, 0.52f);
        c.lineTo(w * 0.90f, 0.06f);
        c.strokePath(col, thick, false);
        c.line(w * 0.48f, 0.52f, w * 0.48f, 0.95f, col, thick);
        return w + 0.13f;
    case 'Z':
        c.beginPath();
        c.moveTo(0.08f, 0.06f);
        c.lineTo(w * 0.88f, 0.06f);
        c.lineTo(0.08f, 0.95f);
        c.lineTo(w * 0.88f, 0.95f);
        c.strokePath(col, thick, false);
        return w + 0.13f;

    case 'a':
        c.beginPath();
        c.moveTo(0.10f, 0.40f);
        c.cubicTo(0.16f, 0.28f, 0.50f, 0.28f, 0.50f, 0.48f);
        c.lineTo(0.50f, 0.95f);
        c.strokePath(col, thick, false);
        c.beginPath();
        c.moveTo(0.50f, 0.60f);
        c.cubicTo(0.24f, 0.58f, 0.06f, 0.66f, 0.06f, 0.80f);
        c.cubicTo(0.06f, 0.96f, 0.34f, 0.96f, 0.50f, 0.84f);
        c.strokePath(col, thick, false);
        return 0.60f * widthScale + 0.09f;
    case 'b':
        c.line(0.08f, 0.05f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.45f);
        c.cubicTo(0.22f, 0.28f, 0.56f, 0.30f, 0.56f, 0.63f);
        c.cubicTo(0.56f, 0.96f, 0.22f, 0.98f, 0.08f, 0.82f);
        c.strokePath(col, thick, false);
        return 0.64f * widthScale + 0.09f;
    case 'd':
        c.line(0.54f, 0.05f, 0.54f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.54f, 0.45f);
        c.cubicTo(0.40f, 0.28f, 0.06f, 0.30f, 0.06f, 0.63f);
        c.cubicTo(0.06f, 0.96f, 0.40f, 0.98f, 0.54f, 0.82f);
        c.strokePath(col, thick, false);
        return 0.64f * widthScale + 0.09f;
    case 'e':
        c.beginPath();
        c.moveTo(0.08f, 0.62f);
        c.lineTo(0.54f, 0.62f);
        c.cubicTo(0.54f, 0.30f, 0.08f, 0.30f, 0.08f, 0.62f);
        c.cubicTo(0.08f, 0.96f, 0.40f, 0.98f, 0.52f, 0.86f);
        c.strokePath(col, thick, false);
        return 0.60f * widthScale + 0.09f;
    case 'i':
        c.line(0.13f, 0.32f, 0.13f, 0.95f, col, thick);
        c.line(0.13f, 0.08f, 0.13f, 0.16f, col, thick);
        return 0.26f * widthScale + 0.07f;
    case 'n':
        c.line(0.08f, 0.32f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.48f);
        c.cubicTo(0.16f, 0.30f, 0.50f, 0.30f, 0.50f, 0.48f);
        c.lineTo(0.50f, 0.95f);
        c.strokePath(col, thick, false);
        return 0.58f * widthScale + 0.09f;
    case 'o':
        c.beginPath();
        c.moveTo(0.31f, 0.32f);
        c.cubicTo(0.56f, 0.32f, 0.56f, 0.95f, 0.31f, 0.95f);
        c.cubicTo(0.06f, 0.95f, 0.06f, 0.32f, 0.31f, 0.32f);
        c.strokePath(col, thick, true);
        return 0.60f * widthScale + 0.09f;
    case 'r':
        c.line(0.08f, 0.32f, 0.08f, 0.95f, col, thick);
        c.beginPath();
        c.moveTo(0.08f, 0.48f);
        c.cubicTo(0.14f, 0.32f, 0.34f, 0.32f, 0.42f, 0.40f);
        c.strokePath(col, thick, false);
        return 0.44f * widthScale + 0.07f;
    case 's':
        c.beginPath();
        c.moveTo(0.48f, 0.40f);
        c.cubicTo(0.40f, 0.30f, 0.08f, 0.30f, 0.08f, 0.46f);
        c.cubicTo(0.08f, 0.63f, 0.50f, 0.63f, 0.50f, 0.80f);
        c.cubicTo(0.50f, 0.96f, 0.12f, 0.96f, 0.06f, 0.84f);
        c.strokePath(col, thick, false);
        return 0.56f * widthScale + 0.09f;
    default:
        return 0.36f * widthScale;
    }
}

float drawVectorString(ImDrawList* dl, float ox, float oy, float charH, float slant,
                       const char* text, ImU32 col, float thick, float widthScale) {
    float x = ox;
    for (const char* p = text; *p; ++p) {
        x += drawVectorChar(dl, x, oy, charH, slant, *p, col, thick, widthScale) * charH;
    }
    return x - ox;
}

} // namespace

void drawJV1010Silkscreen(float targetHeight) {
    // Compact proportions so the full silkscreen fits cleanly on the left (~425 px)
    // and Dummy() reserves the EXACT rendered bounding box (zero overlap with controls).
    const float capH = std::max(20.0f, targetHeight * 0.49f); // ~26.5 px when targetHeight=54
    const float totalH = std::max(targetHeight, capH + 10.0f);

    const float gap1 = capH * 0.52f;
    const float gap2 = capH * 0.38f;
    const float topLineH = capH * 0.38f;
    const float botLineH = capH * 0.42f;
    const float topThick = std::max(1.05f, topLineH * 0.115f);
    const float botThick = std::max(1.25f, botLineH * 0.135f);

    // Pre-calculate exact width of the vector strings so Dummy() reserves the true bounds
    const float rolandW = capH * (87.11f / 18.95f);
    const float jvW = capH * (0.80f + 0.88f + 0.37f + 0.36f + 0.86f + 0.36f + 0.80f);

    // Measure text widths without drawing
    auto measureString = [](const char* text, float charH, float widthScale) {
        ImDrawList* nullDl = nullptr;
        float w = 0.0f;
        const float dummyW = 0.72f * widthScale;
        for (const char* p = text; *p; ++p) {
            char ch = *p;
            if (ch == ' ') w += 0.34f * widthScale * charH;
            else if (ch == '"') w += 0.36f * widthScale * charH;
            else if (ch == '+') w += 0.56f * widthScale * charH;
            else if (ch == 'I') w += (0.28f + 0.12f) * charH;
            else if (ch == 'M') w += (dummyW * 1.04f + 0.14f) * charH;
            else if (ch == 'i') w += (0.26f * widthScale + 0.07f) * charH;
            else if (ch == 'r') w += (0.44f * widthScale + 0.07f) * charH;
            else if (ch >= 'a' && ch <= 'z') w += (0.60f * widthScale + 0.09f) * charH;
            else w += (dummyW + 0.14f) * charH;
        }
        (void)nullDl;
        return w;
    };

    const float topW = measureString("64VOICE SYNTHESIZER MODULE", topLineH, 0.96f);
    const float botW = measureString("\"Session\" onboard + EXPANDABLE", botLineH, 0.82f) + botLineH * 0.25f;
    const float rightTextW = std::max(topW, botW);
    const float exactTotalW = 2.0f + rolandW + gap1 + jvW + gap2 + rightTextW + 4.0f;

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(exactTotalW, totalH));

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Original Roland JV-1010 hardware silkscreen colors
    const ImU32 colRoland = IM_COL32(238, 240, 242, 255); // Crisp silkscreen off-white
    const ImU32 colCyan   = IM_COL32(0, 174, 239, 255);   // Signature JV-1010 cyan-blue
    const ImU32 colCyanLt = IM_COL32(20, 184, 242, 255);  // Upper subtitle cyan

    const float baseY = origin.y + (totalH - capH) * 0.5f;

    // 1. Official Roland wordmark
    float actualRolandW = 0.0f;
    drawOfficialRolandLogo(dl, origin.x + 2.0f, baseY, capH, colRoland, actualRolandW);

    // 2. Crisp filled JV-1010 model designation
    const float jvX = origin.x + 2.0f + actualRolandW + gap1;
    float actualJvW = 0.0f;
    drawJV1010ModelFilled(dl, jvX, baseY, capH, colCyan, actualJvW);

    // 3. Stacked right-hand silkscreen text
    const float infoX = jvX + actualJvW + gap2;
    drawVectorString(dl, infoX, baseY + capH * 0.04f, topLineH, 0.0f,
                     "64VOICE SYNTHESIZER MODULE", colCyanLt, topThick, 0.96f);

    drawVectorString(dl, infoX, baseY + capH * 0.56f, botLineH, 0.23f,
                     "\"Session\" onboard + EXPANDABLE", colCyan, botThick, 0.82f);
}

} // namespace ui
} // namespace jv1010
