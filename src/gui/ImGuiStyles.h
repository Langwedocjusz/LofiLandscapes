#pragma once

#include "imgui.h"

namespace ImGuiStyles{

    void OverShiftedDarkMode() {
        //Based on the dark mode color theme posted by OverShifted:
        //https://github.com/ocornut/imgui/issues/707#issuecomment-678611331

        const ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        const ImVec4 light_grey_lighter = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        //const ImVec4 light_grey         = ImVec4(0.77f, 0.77f, 0.77f, 1.00f);
        const ImVec4 light_grey_darker  = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

        const ImVec4 mid_grey_lighter   = ImVec4(0.50f, 0.50, 0.50f, 1.00f);
        const ImVec4 mid_grey           = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
        const ImVec4 mid_grey_darker    = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);

        const ImVec4 dark_grey_lighter  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        const ImVec4 dark_grey          = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        const ImVec4 dark_grey_darker   = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);
        const ImVec4 dark_grey_darkest  = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);

        const ImVec4 black_half         = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        const ImVec4 black_transparent  = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_Text]                  = white;
        style.Colors[ImGuiCol_TextDisabled]          = mid_grey_lighter;
        style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

        style.Colors[ImGuiCol_WindowBg]              = dark_grey;
        style.Colors[ImGuiCol_ChildBg]               = dark_grey;
        style.Colors[ImGuiCol_PopupBg]               = dark_grey;

        style.Colors[ImGuiCol_Border]                = mid_grey;
        style.Colors[ImGuiCol_BorderShadow]          = black_transparent;

        style.Colors[ImGuiCol_Separator]             = mid_grey;
        style.Colors[ImGuiCol_SeparatorHovered]      = mid_grey;
        style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        style.Colors[ImGuiCol_FrameBg]               = dark_grey_darker;//dark_grey_lighter;
        style.Colors[ImGuiCol_FrameBgHovered]        = dark_grey_darkest;//mid_grey_darker;
        style.Colors[ImGuiCol_FrameBgActive]         = light_grey_darker;

        style.Colors[ImGuiCol_TitleBg]               = dark_grey_darkest;
        style.Colors[ImGuiCol_TitleBgActive]         = dark_grey_darkest;
        style.Colors[ImGuiCol_TitleBgCollapsed]      = black_half;

        style.Colors[ImGuiCol_MenuBarBg]             = dark_grey;

        style.Colors[ImGuiCol_ScrollbarBg]           = dark_grey_darker;//dark_grey_darkest;
        style.Colors[ImGuiCol_ScrollbarGrab]         = dark_grey_lighter;//mid_grey_darker;
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = mid_grey_darker;//mid_grey;
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = light_grey_darker;//mid_grey_lighter;

        style.Colors[ImGuiCol_CheckMark]             = mid_grey_lighter;//light_grey;

        style.Colors[ImGuiCol_SliderGrab]            = dark_grey_lighter;//mid_grey;
        style.Colors[ImGuiCol_SliderGrabActive]      = light_grey_darker;//light_grey;

        style.Colors[ImGuiCol_Button]                = dark_grey_lighter;
        style.Colors[ImGuiCol_ButtonHovered]         = mid_grey_darker;
        style.Colors[ImGuiCol_ButtonActive]          = light_grey_darker;

        style.Colors[ImGuiCol_Header]                = dark_grey_lighter;
        style.Colors[ImGuiCol_HeaderHovered]         = dark_grey_lighter;
        style.Colors[ImGuiCol_HeaderActive]          = light_grey_darker;

        style.Colors[ImGuiCol_ResizeGrip]            = black_transparent;
        style.Colors[ImGuiCol_ResizeGripHovered]     = dark_grey_lighter;
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        style.Colors[ImGuiCol_Tab]                   = dark_grey_darkest;
        style.Colors[ImGuiCol_TabHovered]            = mid_grey_darker;
        style.Colors[ImGuiCol_TabActive]             = dark_grey_lighter;
        style.Colors[ImGuiCol_TabUnfocused]          = dark_grey_darkest;
        style.Colors[ImGuiCol_TabUnfocusedActive]    = dark_grey;

        style.Colors[ImGuiCol_DockingPreview]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        style.Colors[ImGuiCol_DockingEmptyBg]        = dark_grey_lighter;

        style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = white;
        style.Colors[ImGuiCol_NavWindowingDimBg]     = light_grey_lighter;
        style.Colors[ImGuiCol_ModalWindowDimBg]      = light_grey_lighter;

        style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);


        style.GrabRounding      = 1.0f;
        style.FrameRounding     = 1.0f;
        style.WindowRounding    = 1.0f;
        style.ChildRounding     = 1.0f;
        style.FrameRounding     = 1.0f;
        style.PopupRounding     = 1.0f;
        style.ScrollbarRounding = 1.0f;
        style.TabRounding       = 1.0f;
    }
}

/*
        style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);

        style.Colors[ImGuiCol_Border]                = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        style.Colors[ImGuiCol_Separator]             = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

        style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

        style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.77f, 0.77f, 0.77f, 0.49f);

        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.77f, 0.77f, 0.77f, 0.49f);

        style.Colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

        style.Colors[ImGuiCol_Header]                = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);

        style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

        style.Colors[ImGuiCol_Tab]                   = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
        style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
        style.Colors[ImGuiCol_TabActive]             = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);

        style.Colors[ImGuiCol_DockingPreview]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        style.Colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

        style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

        style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
*/
