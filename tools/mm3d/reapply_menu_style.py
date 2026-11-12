#!/usr/bin/env python3
"""Re-apply the styled F1 menu after an upstream sync.

Upstream changes merge in using plain ImGui widgets and tab items. This converts
anything new in the F1 menu section of src/mm3d/overlay.cpp to the MenuStyle
versions, and lists new pages in src/mm3d/menu_style.h. Already-converted code
is left alone, so running it repeatedly is safe.

    python tools/mm3d/reapply_menu_style.py          # apply and report
    python tools/mm3d/reapply_menu_style.py --check  # report only; exit 1 if changes needed
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OVERLAY = ROOT / 'src/mm3d/overlay.cpp'
STYLE = ROOT / 'src/mm3d/menu_style.h'

# F1 menu section: from the styled window setup to the developer editor.
SECTION_START = '        // Player settings cover the whole surface'
SECTION_END = '        if(ImGui::BeginTabBar("Editor settings"))'
# Player page list: after the default tab-bar opening, up to its close.
PAGES_START = '        if(ImGui::BeginTabBar("Settings")) {\n#endif\n'
PAGES_END = '#ifndef MM3D_STYLED_MENU\n            ImGui::EndTabBar();'
WIDGETS = ('Checkbox', 'SliderInt', 'SliderFloat', 'RadioButton', 'Combo')
DIAGNOSTICS_ENTRY = '    {"Diagnostics",'


class MarkerError(RuntimeError):
    pass


def find(text, marker, start=0, name='marker'):
    at = text.find(marker, start)
    if at < 0:
        raise MarkerError(f'Could not find {name}: {marker.strip()!r}. '
                          'The F1 menu was restructured upstream; convert it by hand.')
    return at


def convert_overlay(text):
    """Return (new_text, changes, page_ids) for overlay.cpp."""
    changes = []
    a = find(text, SECTION_START, name='F1 menu start')
    b = find(text, SECTION_END, a, name='F1 menu end')
    section = text[a:b]

    # Plain widgets -> MenuStyle widgets. Only exact calls; signatures match.
    for widget in WIDGETS:
        plain = f'ImGui::{widget}('
        n = section.count(plain)
        if n:
            section = section.replace(plain, f'MenuStyle::{widget}(')
            changes.append(f'{n} x ImGui::{widget} -> MenuStyle::{widget}')

    # SameLine before a Reset button that follows a control (not another button).
    pattern = re.compile(r'(?P<prev>[^\n]*\n)(?P<ind>[ \t]*)ImGui::SameLine\(\);\n'
                         r'(?P<nxt>[ \t]*if\(ImGui::Button\("Reset)')
    count = 0

    def same_line(m):
        nonlocal count
        if 'ImGui::Button(' in m.group('prev'):
            return m.group(0)
        count += 1
        return m.group('prev') + m.group('ind') + 'MenuStyle::AfterControl();\n' + m.group('nxt')
    section = pattern.sub(same_line, section)
    if count:
        changes.append(f'{count} x SameLine before Reset -> MenuStyle::AfterControl')

    # Player page list: tab items -> pages.
    pa = find(section, PAGES_START, name='player page list start') + len(PAGES_START)
    pb = find(section, PAGES_END, pa, name='player page list end')
    pages = section[pa:pb]
    begins = pages.count('ImGui::BeginTabItem(')
    ends = pages.count('ImGui::EndTabItem();')
    if begins or ends:
        pages = pages.replace('ImGui::BeginTabItem(', 'begin_page(').replace('ImGui::EndTabItem();', 'end_page();')
        changes.append(f'{begins} x BeginTabItem / {ends} x EndTabItem -> begin_page / end_page')
    page_ids = re.findall(r'begin_page\("([^"]+)"\)', pages)
    section = section[:pa] + pages + section[pb:]
    return text[:a] + section + text[b:], changes, page_ids


def add_pages(style, page_ids):
    """Insert pages used in overlay.cpp but missing from menu_style.h."""
    known = set(re.findall(r'^\s*\{"([^"]+)",', style, re.M))
    missing = [p for p in dict.fromkeys(page_ids) if p not in known]
    if not missing:
        return style, []
    at = find(style, DIAGNOSTICS_ENTRY, name='Diagnostics page entry')
    lines = ''.join(f'    {{"{p}","{p}",""}},\n' for p in missing)
    return style[:at] + lines + style[at:], [f'added page "{p}" to menu_style.h' for p in missing]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--check', action='store_true', help='report only; exit 1 if changes are needed')
    args = parser.parse_args(argv)
    try:
        overlay, changes, page_ids = convert_overlay(OVERLAY.read_text(encoding='utf-8'))
        style, page_changes = add_pages(STYLE.read_text(encoding='utf-8'), page_ids)
    except MarkerError as error:
        print(f'error: {error}', file=sys.stderr)
        return 2
    changes += page_changes
    if not changes:
        print('F1 menu style is up to date; nothing to change.')
        return 0
    for change in changes:
        print(('needs: ' if args.check else 'changed: ') + change)
    if args.check:
        return 1
    OVERLAY.write_text(overlay, encoding='utf-8')
    STYLE.write_text(style, encoding='utf-8')
    return 0


if __name__ == '__main__':
    sys.exit(main())
