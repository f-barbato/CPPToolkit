#include <cpptoolkit/ui/Application.h>
#include <gtest/gtest.h>

using namespace cpptoolkit::ui;

TEST(StylerTests, PalettesMatchImGuiAndPreserveCustomizedMetrics) {
    Styler styler;
    styler.GetStyle().WindowPadding = ImVec2(19, 23);
    styler.GetStyle().FrameRounding = 7;
    for (const auto theme : {Theme::Dark, Theme::Light, Theme::Classic}) {
        ImGuiStyle expected;
        switch (theme) {
        case Theme::Dark: ImGui::StyleColorsDark(&expected); break;
        case Theme::Light: ImGui::StyleColorsLight(&expected); break;
        case Theme::Classic: ImGui::StyleColorsClassic(&expected); break;
        }
        EXPECT_EQ(&styler.SetTheme(theme), &styler);
        EXPECT_EQ(styler.GetTheme(), theme);
        for (int i = 0; i < ImGuiCol_COUNT; ++i) {
            const auto actual = styler.GetStyle().Colors[i];
            EXPECT_FLOAT_EQ(actual.x, expected.Colors[i].x);
            EXPECT_FLOAT_EQ(actual.y, expected.Colors[i].y);
            EXPECT_FLOAT_EQ(actual.z, expected.Colors[i].z);
            EXPECT_FLOAT_EQ(actual.w, expected.Colors[i].w);
        }
        EXPECT_FLOAT_EQ(styler.GetStyle().WindowPadding.x, 19);
        EXPECT_FLOAT_EQ(styler.GetStyle().FrameRounding, 7);
    }
}

TEST(StylerTests, PaletteDrivesWidgetColorsAndPreservesMetrics) {
    Styler styler;
    styler.GetStyle().FrameRounding = 7;
    Palette palette = Palette::Light();
    palette.Primary = ImVec4(1, 0, 0, 1);
    palette.Background = ImVec4(0.2f, 0.3f, 0.4f, 1);
    EXPECT_EQ(&styler.SetPalette(palette), &styler);
    const auto& colors = styler.GetStyle().Colors;
    EXPECT_FLOAT_EQ(colors[ImGuiCol_WindowBg].z, 0.4f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_ButtonHovered].x, 1.0f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_CheckMark].y, 0.0f);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_Text].x, palette.Text.x);
    EXPECT_FLOAT_EQ(colors[ImGuiCol_ChildBg].w, 0.0f);
    EXPECT_FLOAT_EQ(styler.GetStyle().FrameRounding, 7);
    EXPECT_FLOAT_EQ(styler.GetPalette().Primary.x, 1.0f);
}

TEST(StylerTests, ApplyRequiresContextAndCopiesAllCustomizations) {
    Styler styler(Theme::Classic);
    styler.GetStyle().WindowRounding = 9;
    styler.GetStyle().FrameBorderSize = 2;
    styler.GetStyle().ItemSpacing = ImVec2(11, 17);
    styler.GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.2f, 0.3f, 1);
    EXPECT_THROW(styler.Apply(), std::logic_error);
    ImGui::CreateContext();
    styler.Apply();
    EXPECT_FLOAT_EQ(ImGui::GetStyle().WindowRounding, 9);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FrameBorderSize, 2);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().ItemSpacing.y, 17);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].z, 0.3f);
    ImGui::DestroyContext();
}

TEST(StylerTests, ApplicationOwnsSnapshotAndLegacyThemePreservesMetrics) {
    Application application;
    EXPECT_FALSE(application.IsDarkTheme());
    Styler styler(Theme::Classic);
    styler.GetStyle().WindowRounding = 8;
    EXPECT_EQ(&application.SetStyler(styler), &application);
    styler.GetStyle().WindowRounding = 99;
    EXPECT_FLOAT_EQ(application.GetStyler().GetStyle().WindowRounding, 8);
    EXPECT_EQ(application.GetStyler().GetTheme(), Theme::Classic);
    application.SetDarkTheme(true);
    EXPECT_TRUE(application.IsDarkTheme());
    EXPECT_FLOAT_EQ(application.GetStyler().GetStyle().WindowRounding, 8);
    application.SetDarkTheme(false);
    EXPECT_EQ(application.GetStyler().GetTheme(), Theme::Light);
}

TEST(StylerTests, ApplicationValidatesAndStoresFontConfiguration) {
    Application application;
    EXPECT_TRUE(application.GetFontPath().empty());
    EXPECT_THROW(application.SetFont("missing-font.ttf", 16), std::invalid_argument);
    EXPECT_THROW(application.SetFont(__FILE__, 0), std::invalid_argument);
    EXPECT_EQ(&application.SetFont(__FILE__, 18), &application);
    EXPECT_EQ(application.GetFontPath(), __FILE__);
    EXPECT_FLOAT_EQ(application.GetFontSize(), 18);
    EXPECT_EQ(&application.ResetFont(), &application);
    EXPECT_TRUE(application.GetFontPath().empty());
    EXPECT_FLOAT_EQ(application.GetFontSize(), 0);
}
