import unittest

import reapply_menu_style as r

SAMPLE = (
    '        // Player settings cover the whole surface; text size never moves the panel.\n'
    '        const auto slider=[&]{\n'
    '            ImGui::SliderInt("Size",&v,0,9);\n'
    '            ImGui::SameLine();\n'
    '            if(ImGui::Button("Reset size")){}\n'
    '            if(ImGui::Button("Reset position")){}\n'
    '            ImGui::SameLine();\n'
    '            if(ImGui::Button("Reset element")){}\n'
    '        };\n'
    '        if(ImGui::BeginTabBar("HUD options")){if(ImGui::BeginTabItem("Elements")){ImGui::EndTabItem();}}\n'
    '        if(ImGui::BeginTabBar("Settings")) {\n#endif\n'
    '            if(begin_page("General")){end_page();}\n'
    '            if(ImGui::BeginTabItem("Network")){ImGui::Checkbox("On",&on);ImGui::EndTabItem();}\n'
    '#ifndef MM3D_STYLED_MENU\n            ImGui::EndTabBar();\n#endif\n'
    '        if(ImGui::BeginTabBar("Editor settings")) {ImGui::Checkbox("Editor",&e);}\n'
)
STYLE = ('inline constexpr PageInfo Pages[]={\n    {"General","General","x"},\n'
         '    {"Diagnostics","Diagnostics","y"},\n};\n')


class ReapplyMenuStyleTest(unittest.TestCase):
    def test_converts_new_upstream_code_once(self):
        text, changes, pages = r.convert_overlay(SAMPLE)
        self.assertIn('MenuStyle::SliderInt("Size"', text)
        self.assertIn('MenuStyle::Checkbox("On"', text)
        self.assertIn('if(begin_page("Network")){', text)
        self.assertIn('end_page();}', text)
        self.assertEqual(text.count('MenuStyle::AfterControl();'), 1)  # not between two buttons
        self.assertEqual(pages, ['General', 'Network'])
        self.assertTrue(changes)
        again, changes, _ = r.convert_overlay(text)
        self.assertEqual(again, text)
        self.assertEqual(changes, [])

    def test_leaves_nested_tabs_and_editor_alone(self):
        text, _, _ = r.convert_overlay(SAMPLE)
        self.assertIn('ImGui::BeginTabItem("Elements")', text)
        self.assertIn('ImGui::Checkbox("Editor"', text)

    def test_adds_missing_pages_before_diagnostics(self):
        style, changes = r.add_pages(STYLE, ['General', 'Network'])
        self.assertLess(style.index('"Network"'), style.index('"Diagnostics"'))
        self.assertEqual(len(changes), 1)
        self.assertEqual(r.add_pages(style, ['General', 'Network'])[1], [])

    def test_reports_restructured_menu(self):
        with self.assertRaises(r.MarkerError):
            r.convert_overlay('no menu here')


if __name__ == '__main__':
    unittest.main()
