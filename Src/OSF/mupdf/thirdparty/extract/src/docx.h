#ifndef ARTIFEX_EXTRACT_DOCX_H
#define ARTIFEX_EXTRACT_DOCX_H

/* Only for internal use by extract code.  */

/* Things for creating docx files. */

int extract_document_to_docx_content(
        document_t*         document,
        int                 spacing,
        int                 rotation,
        int                 images,
        extract_astring_t*  content
        );
/* Makes *o_content point to a string containing all paragraphs in *document in
docx XML format.

This string can be passed to extract_docx_content_item() or
extract_docx_write_template() to be inserted into a docx archive's
word/document.xml. */


int extract_docx_write_template(
        extract_astring_t*  contentss,
        int                 contentss_num,
        images_t*           images,
        const char*         path_template,
        const char*         path_out,
        int                 preserve_dir
        );
/* Creates a new docx file using a provided template document.

Uses the 'zip' and 'unzip' commands internally.

contents
contentss_num
    Content to be inserted into word/document.xml.
document
    .
images
    Information about images.
path_template
    Name of docx file to use as a template.
path_out
    Name of docx file to create. Must not contain single-quote, double quote,
    space or ".." sequence - these will force EINVAL error because they could
    make internal shell commands unsafe.
preserve_dir
    If true, we don't delete the temporary directory <path_out>.dir containing
    unzipped docx content.
*/


int extract_docx_content_item(
        extract_astring_t*  contentss,
        int                 contentss_num,
        images_t*           images,
        const char*         name,
        const char*         text,
        char**              text2
        );
/* Determines content of <name> in docx archive.

content
content_length
    Text to insert if <name> is word/document.xml.
images
    Information about images. If <name> is word/document.xml we insert
    relationship information mapping from image ids to image names;
    <text> should already contain reference ids for images. If <name> is
    [Content_Types].xml we insert information about image types.
name
    Path within the docx zip archive.
text
    Content of <name> in template docx file.
text2
    Out-param. Set to NULL if <text> should be used unchanged. Otherwise set to
    point to desired text, allocated with malloc() which caller should free.
*/

#endif
