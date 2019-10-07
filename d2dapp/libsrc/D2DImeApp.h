#pragma once

void App_OnInputLanguageChanged(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextServicesManager^ sender, Platform::Object^ args);
void App_OnTextUpdating( D2CoreTextBridge& bridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextUpdatingEventArgs^ args);

void App_OnFormatUpdating(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextFormatUpdatingEventArgs^ args);

void App_OnCompositionStarted(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionStartedEventArgs^ args);
void App_OnCompositionCompleted(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextCompositionCompletedEventArgs^ args);

void App_OnFocusRemoved(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Platform::Object^ args);

void App_OnLayoutRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextLayoutRequestedEventArgs^ args);


void App_OnSelectionRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionRequestedEventArgs^ args);

void App_OnSelectionUpdating(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextSelectionUpdatingEventArgs^ args);
void App_OnTextRequested(D2CoreTextBridge& imeBridge, Windows::UI::Text::Core::CoreTextEditContext^ sender, Windows::UI::Text::Core::CoreTextTextRequestedEventArgs^ args);







