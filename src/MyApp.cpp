// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP

#include <wx/wx.h>
#include <wx/tokenzr.h>

#define __GXX_ABI_VERSION 1002

#define ASSERT_IMAGE_OPEN if (!image) {\
        wxLogMessage("You must open an image first!");\
        return;\
    }

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

    void OnShowCumulativeHistogram(wxCommandEvent &event);

    void OnAdjustBrightness(wxCommandEvent &event);

    void OnAdjustContrast(wxCommandEvent &event);

    void OnNegative(wxCommandEvent &event);

    void OnEqualizeHistogram(wxCommandEvent &event);

    void OnMatchHistogram(wxCommandEvent &event);

    void OnZoomOut(wxCommandEvent &event);

    void OnZoomIn(wxCommandEvent &event);

    void OnExit(wxCommandEvent &event);

    void OnAbout(wxCommandEvent &event);

    void ShowImage();

    void ShowImageInNewFrame(image_t *image_to_show, const char *frame_title);
};

enum {
    ID_OPEN = 1,
    ID_SAVE = 2,
    ID_MIRROR_VERTICALLY = 3,
    ID_MIRROR_HORIZONTALLY = 4,
    ID_GRAY_SCALE = 5,
    ID_QUANTIZE = 6,
    ID_SHOW_HISTOGRAM = 7,
    ID_SHOW_CUM_HISTOGRAM = 8,
    ID_ADJUST_BRIGHTNESS = 9,
    ID_ADJUST_CONTRAST = 10,
    ID_NEGATIVE = 11,
    ID_EQUALIZE_HISTOGRAM = 12,
    ID_MATCH_HISTOGRAM = 13,
    ID_ZOOM_OUT = 14,
    ID_ZOOM_IN = 15
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
    menu2->Append(ID_SHOW_CUM_HISTOGRAM, "&Show Cumulative Histogram...\tCtrl-Shift-H",
                  "Calculate and show histogram");
    menu2->Append(ID_ADJUST_BRIGHTNESS, "&Adjust Brightness...\tCtrl-B",
                  "Add bias term to image");
    menu2->Append(ID_ADJUST_CONTRAST, "&Adjust Contrast...\tCtrl-C",
                  "Multiply gain term to image");
    menu2->Append(ID_NEGATIVE, "&Negative...\tCtrl-N",
                  "Make image it's negative (p' = 255 - p)");
    menu2->Append(ID_EQUALIZE_HISTOGRAM, "&Equalize Histogram...\tCtrl-E",
                  "Attempts to optimize contrast with histogram equalization");
    menu2->Append(ID_MATCH_HISTOGRAM, "&Match Histogram...\tCtrl-Alt-M",
                  "Attempts to match current image's histogram to target's histogram");
    menu2->Append(ID_ZOOM_OUT, "&Zoom Out...\tCtrl-PageDown",
                  "Zooms out on the image using a window of size Sx by Sy");
    menu2->Append(ID_ZOOM_IN, "&Zoom In...\tCtrl-PageUp",
                  "Zooms in on the image by a factor of 2x2");

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
    Bind(wxEVT_MENU, &MyFrame::OnShowCumulativeHistogram, this, ID_SHOW_CUM_HISTOGRAM);
    Bind(wxEVT_MENU, &MyFrame::OnAdjustBrightness, this, ID_ADJUST_BRIGHTNESS);
    Bind(wxEVT_MENU, &MyFrame::OnAdjustContrast, this, ID_ADJUST_CONTRAST);
    Bind(wxEVT_MENU, &MyFrame::OnNegative, this, ID_NEGATIVE);
    Bind(wxEVT_MENU, &MyFrame::OnEqualizeHistogram, this, ID_EQUALIZE_HISTOGRAM);
    Bind(wxEVT_MENU, &MyFrame::OnMatchHistogram, this, ID_MATCH_HISTOGRAM);
    Bind(wxEVT_MENU, &MyFrame::OnZoomOut, this, ID_ZOOM_OUT);
    Bind(wxEVT_MENU, &MyFrame::OnZoomIn, this, ID_ZOOM_IN);

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

    // Creates a "open file" dialog with 2 file types
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
    ASSERT_IMAGE_OPEN

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
    ASSERT_IMAGE_OPEN

    mirror_vertically(image);

    ShowImage();
}

void MyFrame::OnMirrorHorizontally(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    mirror_horizontally(image);

    ShowImage();
}

void MyFrame::OnGrayScale(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    rgb_to_luminance(image);

    ShowImage();
}

void MyFrame::OnQuantize(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

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
    ASSERT_IMAGE_OPEN

    image_t *displayable = get_displayable(image);
    wxImage wx_image(displayable->width, displayable->height, pixel_array_to_unsigned_char_array(displayable), true);
    wxBitmap wx_bitmap(wx_image);

    // Update later with your bitmap
    staticBitmap->SetBitmap(wx_bitmap);
}

void MyFrame::ShowImageInNewFrame(image_t *image_to_show, const char *frame_title) {
    wxFrame *frame = new wxFrame(nullptr, histogramFrames++, frame_title, wxPoint(50, 50),
                                 wxSize(static_cast<int>(image_to_show->height * 1.2),
                                        static_cast<int>(image_to_show->width * 1.2)));
    wxPanel *panel = new wxPanel(frame);
    auto *hbox = new wxBoxSizer(wxHORIZONTAL);

    auto *histogramBitmap = new wxStaticBitmap(frame, wxID_STATIC, wxNullBitmap, wxDefaultPosition,
                                               wxSize(HISTOGRAM_SIZE, HISTOGRAM_SIZE));

    hbox->Add(histogramBitmap, 1, wxEXPAND);
    frame->SetSizer(hbox);

    image_t *displayable = get_displayable(image_to_show);
    wxImage wx_image(displayable->width, displayable->height, pixel_array_to_unsigned_char_array(displayable), true);
    wxBitmap wx_bitmap(wx_image);

    frame->Show(true);
    histogramBitmap->SetBitmap(wx_bitmap);
}

void MyFrame::OnShowHistogram(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    image_t *histogram = histogram_plot(compute_histogram(image));
    if (!histogram) {
        wxLogMessage("Error generating histogram");
        return;
    }

    ShowImageInNewFrame(histogram, "Histogram");
}

void MyFrame::OnShowCumulativeHistogram(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    image_t *cum_hist = histogram_plot(compute_norm_cum_histogram(image));
    if (!cum_hist) {
        wxLogMessage("Error generating histogram");
        return;
    }
    const char *frame_title = "Cumulative Histogram";

    ShowImageInNewFrame(cum_hist, frame_title);
}

void MyFrame::OnAdjustBrightness(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

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
    ASSERT_IMAGE_OPEN

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
    ASSERT_IMAGE_OPEN

    negative(image);

    ShowImage();
}

void MyFrame::OnEqualizeHistogram(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    equalize_histogram(image);

    ShowImage();
}

void MyFrame::OnMatchHistogram(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    wxFileDialog *OpenDialog = new wxFileDialog(
            this, _("Choose histogram matching target"), wxEmptyString, wxEmptyString,
            _("JPEG images (*.jpg, *.jpeg)|*.jpg;*.jpeg"),
            wxFD_OPEN, wxDefaultPosition);

    // Creates a "open file" dialog with 2 file types
    if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        wxString filename = OpenDialog->GetPath();

        image_t *target = jpeg_decompress((char *) filename.mb_str().data());
        // Set the Status to reflect that file saved
        SetStatusText((image->last_operation == DECOMPRESSION_SUCCESS) ? "File opened successfully!"
                                                                       : "Failed to open file!");

        if (image->last_operation == DECOMPRESSION_SUCCESS) {

            image_t *histogram_source = histogram_plot(compute_histogram(image));
            image_t *histogram_target = histogram_plot(compute_histogram(target));
            ShowImageInNewFrame(histogram_source, "Source Histogram");
            ShowImageInNewFrame(histogram_target, "Target Histogram");

            match_histogram(image, target);

            image_t *histogram_matched = histogram_plot(compute_histogram(image));
            ShowImageInNewFrame(histogram_matched, "Matched Histogram");

            ShowImage();
        }
    }
}

void MyFrame::OnZoomOut(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN;

    wxTextEntryDialog *TextEntryDialog = new wxTextEntryDialog(
            this, _("Provide Sx,Sy (size of zoom-out window)"), _("Zoom out"));

    if (TextEntryDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
    {
        double sx = 0, sy = 0;
        bool bad_input = false;
        wxStringTokenizer tokenizer(TextEntryDialog->GetValue(), ",");

        if (tokenizer.HasMoreTokens()) tokenizer.NextToken().ToDouble(&sx);
        else bad_input = true;

        if (tokenizer.HasMoreTokens()) tokenizer.NextToken().ToDouble(&sy);
        else bad_input = true;

        if (bad_input || sx < 1 || sy < 1) {
            wxLogMessage("Provide Sx and Sy >= 1. Use the format Sx,Sy.");
            return;
        }

        zoom_out(image, static_cast<int>(sx), static_cast<int>(sy));

        ShowImage();
    }
}

void MyFrame::OnZoomIn(wxCommandEvent &event) {
    ASSERT_IMAGE_OPEN

    zoom_in(image);

    ShowImage();
}