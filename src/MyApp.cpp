// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP

#include <wx/wx.h>

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

    void OnOpen(wxCommandEvent &event);

    void OnSave(wxCommandEvent &event);

    void OnExit(wxCommandEvent &event);

    void OnAbout(wxCommandEvent &event);
};

enum {
    ID_OPEN = 1,
    ID_SAVE = 2
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
        : wxFrame(NULL, wxID_ANY, "Hello World") {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_OPEN, "&Open...\tCtrl-O",
                     "Open an image from internal storage");
    menuFile->Append(ID_SAVE, "&Save...\tCtrl-S",
                     "Save edited image to internal storage");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to FPI Assignment 1!");
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_OPEN);
    Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_SAVE);
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
    }
    //wxLogMessage("Hello world from FPI Assignment 1!");
}

void MyFrame::OnSave(wxCommandEvent &event) {
    wxFileDialog *SaveDialog = new wxFileDialog(
            this, _("Choose where to save the file"), wxEmptyString, wxEmptyString,
            _("JPEG images (*.jpg, *.jpeg)|*.jpg;*.jpeg"),
            wxFD_SAVE, wxDefaultPosition);

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