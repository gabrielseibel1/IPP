// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP

#include <wx/wx.h>

#define __GXX_ABI_VERSION 1002

extern "C" {
#include <image_manipulation.h>
};

#endif

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame();

private:
    image_t *image;

    int histogramFrames = 0;

    wxStaticBitmap *staticBitmap;

    void OnOpen(wxCommandEvent &event);

    void OnSave(wxCommandEvent &event);

    void OnMirrorVertically(wxCommandEvent &event);

    void OnMirrorHorizontally(wxCommandEvent &event);

    void OnGrayScale(wxCommandEvent &event);

    void OnQuantize(wxCommandEvent &event);

    void OnShowHistogram(wxCommandEvent &event);

    void OnAdjustBrightness(wxCommandEvent &event);

    void OnAdjustContrast(wxCommandEvent &event);

    void OnNegative(wxCommandEvent &event);

    void OnExit(wxCommandEvent &event);

    void OnAbout(wxCommandEvent &event);

    void ShowImage();
};

enum {
    ID_OPEN = 1,
    ID_SAVE = 2,
    ID_MIRROR_VERTICALLY = 3,
    ID_MIRROR_HORIZONTALLY = 4,
    ID_GRAY_SCALE = 5,
    ID_QUANTIZE = 6,
    ID_SHOW_HISTOGRAM = 7,
    ID_ADJUST_BRIGHTNESS = 8,
    ID_ADJUST_CONTRAST = 9,
    ID_NEGATIVE = 10
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
        : wxFrame(NULL, wxID_ANY, "FPI Assignment 1", wxPoint(-1, -1), wxSize(600, 600)) {
    image = nullptr;

    auto *menuFile = new wxMenu;
    menuFile->Append(ID_OPEN, "&Open...\tCtrl-O",
                     "Open an image from internal storage");
    menuFile->Append(ID_SAVE, "&Save...\tCtrl-S",
                     "Save edited image to internal storage");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    auto *menu1 = new wxMenu;
    menu1->Append(ID_MIRROR_VERTICALLY, "&Mirror Vertically...\tCtrl-M",
                  "Mirror the image in the up/down direction");
    menu1->Append(ID_MIRROR_HORIZONTALLY, "&Mirror Horizontally...\tCtrl-Shift-M",
                  "Mirror the image in the left/right direction");
    menu1->Append(ID_GRAY_SCALE, "&To Gray Scale...\tCtrl-G",
                  "Apply L = 0.299*R + 0.587*G + 0.114*B , Ri = Gi = Bi = Li");
    menu1->Append(ID_QUANTIZE, "&Quantize...\tCtrl-Q",
                  "Apply tone quantization");

    auto *menu2 = new wxMenu;
    menu2->Append(ID_SHOW_HISTOGRAM, "&Show Histogram...\tCtrl-H",
                  "Calculate and show histogram");
    menu2->Append(ID_ADJUST_BRIGHTNESS, "&Adjust Brightness...\tCtrl-B",
                  "Add bias term to image");
    menu2->Append(ID_ADJUST_CONTRAST, "&Adjust Contrast...\tCtrl-C",
                  "Multiply gain term to image");
    menu2->Append(ID_NEGATIVE, "&Negative...\tCtrl-N",
                  "Make image it's negative (p' = 255 - p)");

    auto *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    auto *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menu1, "&Assignment 1");
    menuBar->Append(menu2, "&Assignment 2");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to Image Processing Playground!");

    wxPanel *panel = new wxPanel(this, -1);
    auto *hbox = new wxBoxSizer(wxHORIZONTAL);

    staticBitmap = new wxStaticBitmap(this, wxID_STATIC, wxNullBitmap, wxDefaultPosition,
                                      wxSize(200, 200));

    hbox->Add(staticBitmap, 1, wxALL | wxEXPAND, 15);
    panel->SetSizer(hbox);
    Centre();

    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_OPEN);
    Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_SAVE);
    Bind(wxEVT_MENU, &MyFrame::OnMirrorVertically, this, ID_MIRROR_VERTICALLY);
    Bind(wxEVT_MENU, &MyFrame::OnMirrorHorizontally, this, ID_MIRROR_HORIZONTALLY);
    Bind(wxEVT_MENU, &MyFrame::OnGrayScale, this, ID_GRAY_SCALE);
    Bind(wxEVT_MENU, &MyFrame::OnQuantize, this, ID_QUANTIZE);
    Bind(wxEVT_MENU, &MyFrame::OnShowHistogram, this, ID_SHOW_HISTOGRAM);
    Bind(wxEVT_MENU, &MyFrame::OnAdjustBrightness, this, ID_ADJUST_BRIGHTNESS);
    Bind(wxEVT_MENU, &MyFrame::OnAdjustContrast, this, ID_ADJUST_CONTRAST);
    Bind(wxEVT_MENU, &MyFrame::OnNegative, this, ID_NEGATIVE);

    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent &event) {
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event) {
    wxMessageBox("This is a simple image manipulation tool\n"
                 "built with jpeglib and wxWidgets by Gabriel Seibel\n",
                 "About FPI Assignment 1", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnOpen(wxCommandEvent &event) {
    wxFileDialog *OpenDialog = new wxFileDialog(
            this, _("Choose a file to open"), wxEmptyString, wxEmptyString,
            _("JPEG images (*.jpg, *.jpeg)|*.jpg;*.jpeg"),
            wxFD_OPEN, wxDefaultPosition);

    // Creates a "open file" dialog with 4 file types
    if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        wxString filename = OpenDialog->GetPath();

        // Sets our current document to the file the user selected
        image = jpeg_decompress((char *) filename.mb_str().data());
        // Set the Title to reflect the  file open
        SetTitle(wxString("Edit - ") << OpenDialog->GetFilename());
        // Set the Status to reflect that file saved
        SetStatusText((image->last_operation == DECOMPRESSION_SUCCESS) ? "File opened successfully!"
                                                                       : "Failed to open file!");

        if (image->last_operation == DECOMPRESSION_SUCCESS) {
            ShowImage();
        }
    }

}

void MyFrame::OnSave(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    wxFileDialog *SaveDialog = new wxFileDialog(
            this, _("Choose where to save the file"), wxEmptyString, wxEmptyString,
            _("JPEG images (*.jpg, *.jpeg)|*.jpg;*.jpeg"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

    // Creates a "open file" dialog with 4 file types
    if (SaveDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        wxString filename = SaveDialog->GetPath();

        // Sets our current document to the file the user selected
        jpeg_compress(image, (char *) filename.mb_str().data());

        // Set the Status to reflect that file saved
        SetStatusText(
                (image->last_operation == COMPRESSION_SUCCESS) ? "File saved successfully!" : "Failed to save file!");
    }
}

void MyFrame::OnMirrorVertically(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    mirror_vertically(image);

    ShowImage();
}

void MyFrame::OnMirrorHorizontally(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    mirror_horizontally(image);

    ShowImage();
}

void MyFrame::OnGrayScale(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    rgb_to_luminance(image);

    ShowImage();
}

void MyFrame::OnQuantize(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    wxTextEntryDialog *TextEntryDialog = new wxTextEntryDialog(
            this, _("Number of tones"), _("Quantization"));

    if (TextEntryDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        double tones;
        TextEntryDialog->GetValue().ToDouble(&tones);
        quantize(image, (int) tones);

        ShowImage();
    }
}

void MyFrame::ShowImage() {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    image_t *displayable = get_displayable(image);
    wxImage wx_image(displayable->width, displayable->height, pixel_array_to_unsigned_char_array(displayable), true);
    wxBitmap wx_bitmap(wx_image);

    // Update later with your bitmap
    staticBitmap->SetBitmap(wx_bitmap);
}

void MyFrame::OnShowHistogram(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    wxFrame *frame = new wxFrame(nullptr, histogramFrames++, "Histogram", wxPoint(50, 50),
                                 wxSize(static_cast<int>(HISTOGRAM_SIZE * 1.2), static_cast<int>(HISTOGRAM_SIZE * 1.2)));
    wxPanel *panel = new wxPanel(frame);
    auto *hbox = new wxBoxSizer(wxHORIZONTAL);

    auto *histogramBitmap = new wxStaticBitmap(frame, wxID_STATIC, wxNullBitmap, wxDefaultPosition,
                                               wxSize(HISTOGRAM_SIZE, HISTOGRAM_SIZE));

    hbox->Add(histogramBitmap, 1, wxEXPAND);
    frame->SetSizer(hbox);

    image_t *histogram = histogram_plot(compute_histogram(image));

    if (!histogram) {
        wxLogMessage("Error generating histogram");
        return;
    }

    image_t *displayable = get_displayable(histogram);
    wxImage wx_image(displayable->width, displayable->height, pixel_array_to_unsigned_char_array(displayable), true);
    wxBitmap wx_bitmap(wx_image);


    frame->Show(true);
    histogramBitmap->SetBitmap(wx_bitmap);
}

void MyFrame::OnAdjustBrightness(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    wxTextEntryDialog *TextEntryDialog = new wxTextEntryDialog(
            this, _("Summing bias term value"), _("Brightness Adjust"));

    if (TextEntryDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        double bias;
        TextEntryDialog->GetValue().ToDouble(&bias);

        if (bias >= -255 && bias <= 255) {
            add_bias(image, bias);
        } else {
            wxLogMessage("Enter a value in the range [0,255]");
            return;
        }

        ShowImage();
    }
}

void MyFrame::OnAdjustContrast(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    wxTextEntryDialog *TextEntryDialog = new wxTextEntryDialog(
            this, _("Multiplying gain term value"), _("Contrast Adjustment"));

    if (TextEntryDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        double gain;
        TextEntryDialog->GetValue().ToDouble(&gain);

        if (gain > 0 && gain <= 255) {
            multiply_gain(image, gain);
        } else {
            wxLogMessage("Enter a value in the range (0,255]");
            return;
        }

        ShowImage();
    }
}

void MyFrame::OnNegative(wxCommandEvent &event) {
    if (!image) {
        wxLogMessage("You must open an image first!");
        return;
    }

    negative(image);

    ShowImage();
}