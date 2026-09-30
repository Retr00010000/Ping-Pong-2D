import os
import sys
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.pdfgen import canvas

# =======================================================================
# CUSTOM NUMBERED CANVAS FOR HEADER & FOOTER
# =======================================================================
class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super(NumberedCanvas, self).showPage()
        super(NumberedCanvas, self).save()

    def draw_page_decorations(self, page_count):
        # Don't draw header/footer on cover page (page 1)
        if self._pageNumber == 1:
            return

        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748b"))

        # Running Header
        self.drawString(54, letter[1] - 36, "PONG 2D & MODERN OPENGL ARCHITECTURE GUIDE")
        self.setStrokeColor(colors.HexColor("#cbd5e1"))
        self.setLineWidth(0.5)
        self.line(54, letter[1] - 42, letter[0] - 54, letter[1] - 42)

        # Running Footer
        self.line(54, 45, letter[0] - 54, 45)
        self.drawString(54, 32, "Confidential & Educational — 2D Engine Systems & Modern Graphics Pipeline")
        page_str = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(letter[0] - 54, 32, page_str)

        self.restoreState()


def create_callout(title, text, style_title, style_body, bg_color="#f1f5f9", border_color="#0284c7"):
    content = [
        Paragraph(f"<b>{title}</b>", style_title),
        Spacer(1, 4),
        Paragraph(text, style_body)
    ]
    t = Table([[content]], colWidths=[letter[0] - 108])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor(bg_color)),
        ('LINELEFT', (0,0), (-1,-1), 4, colors.HexColor(border_color)),
        ('TOPPADDING', (0,0), (-1,-1), 8),
        ('BOTTOMPADDING', (0,0), (-1,-1), 8),
        ('LEFTPADDING', (0,0), (-1,-1), 12),
        ('RIGHTPADDING', (0,0), (-1,-1), 12),
    ]))
    return t

def create_code_block(code_text, style_code, bg_color="#0f172a"):
    escaped = code_text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\n", "<br/>").replace(" ", "&nbsp;")
    p = Paragraph(f"<font face='Courier' color='#f8fafc'>{escaped}</font>", style_code)
    t = Table([[p]], colWidths=[letter[0] - 108])
    t.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor(bg_color)),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor("#334155")),
        ('TOPPADDING', (0,0), (-1,-1), 8),
        ('BOTTOMPADDING', (0,0), (-1,-1), 8),
        ('LEFTPADDING', (0,0), (-1,-1), 10),
        ('RIGHTPADDING', (0,0), (-1,-1), 10),
    ]))
    return t


def build_book():
    pdf_filename = "Pong_2D_Architecture_Book.pdf"
    doc = SimpleDocTemplate(
        pdf_filename,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()

    # Custom Typography Styles
    title_style = ParagraphStyle(
        'CoverTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=28,
        leading=34,
        textColor=colors.HexColor('#0f172a'),
        alignment=0,
        spaceAfter=10
    )

    subtitle_style = ParagraphStyle(
        'CoverSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=13,
        leading=18,
        textColor=colors.HexColor('#475569'),
        spaceAfter=25
    )

    meta_style = ParagraphStyle(
        'CoverMeta',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=14,
        textColor=colors.HexColor('#0284c7')
    )

    h1_style = ParagraphStyle(
        'CustomH1',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=18,
        leading=22,
        textColor=colors.HexColor('#0f172a'),
        spaceBefore=18,
        spaceAfter=10,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'CustomH2',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=13,
        leading=17,
        textColor=colors.HexColor('#0369a1'),
        spaceBefore=12,
        spaceAfter=6,
        keepWithNext=True
    )

    h3_style = ParagraphStyle(
        'CustomH3',
        parent=styles['Heading3'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=14,
        textColor=colors.HexColor('#1e293b'),
        spaceBefore=8,
        spaceAfter=4,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'CustomBody',
        parent=styles['BodyText'],
        fontName='Helvetica',
        fontSize=9.5,
        leading=14,
        textColor=colors.HexColor('#334155'),
        spaceAfter=8
    )

    callout_title = ParagraphStyle(
        'CalloutTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10,
        leading=13,
        textColor=colors.HexColor('#0369a1')
    )

    callout_body = ParagraphStyle(
        'CalloutBody',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9,
        leading=13,
        textColor=colors.HexColor('#334155')
    )

    code_style = ParagraphStyle(
        'CodeStyle',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=8,
        leading=11
    )

    story = []

    # =======================================================================
    # COVER PAGE
    # =======================================================================
    story.append(Spacer(1, 40))
    story.append(Paragraph("TECHNICAL REFERENCE MANUAL", meta_style))
    story.append(Spacer(1, 10))
    story.append(Paragraph("THE DEFINITIVE ARCHITECTURE GUIDE:<br/>2D PONG IN MODERN OPENGL", title_style))
    story.append(HRFlowable(width="100%", thickness=3, color=colors.HexColor('#0284c7'), spaceAfter=15, spaceBefore=5))
    story.append(Paragraph(
        "A Comprehensive Deep Dive into Core Profile Graphics, Buffer Objects (VBO, VAO, EBO), "
        "Custom Engine Abstractions, Shaders, Bitmapped Typography, Real-time Physics, and Systems Engineering.",
        subtitle_style
    ))

    # Cover Summary Box
    cover_box_text = (
        "<b>What This Mini-Book Covers:</b><br/>"
        "• <b>The Modern Graphics Pipeline:</b> How CPU and GPU cooperate in OpenGL 4.6 Core Profile.<br/>"
        "• <b>The Buffer Trinity (VBO, VAO, EBO):</b> Exact memory layouts, stride calculations, attribute binding mechanics, and index caching.<br/>"
        "• <b>Custom Engine Architecture:</b> Line-by-line breakdown of VAO, VBO, EBO, Shader, and Texture classes.<br/>"
        "• <b>The Unit Quad Paradigm:</b> How a single 4-vertex geometry renders all paddles, ball, net, and text via shader uniforms.<br/>"
        "• <b>5x3 Retro Bitmap Font Engine:</b> Bitwise bitmask extraction decoding retro characters without external font libraries.<br/>"
        "• <b>Dynamic Game Physics:</b> Trigonometric angle reflection, compounding velocity acceleration, and anti-tunneling math.<br/>"
        "• <b>Game Loop & Concurrency:</b> GLFW windowing, Delta Time clamping, state debouncing, and asynchronous audio."
    )
    story.append(create_callout("ABOUT THIS MANUAL", cover_box_text, callout_title, callout_body, "#f0f9ff", "#0284c7"))
    story.append(Spacer(1, 80))

    meta_footer = (
        "<b>Project:</b> Ping_Pong_2D &nbsp;|&nbsp; <b>Language:</b> C++20 / GLSL 460 &nbsp;|&nbsp; <b>Libraries:</b> GLFW 3, GLAD, stb_image<br/>"
        "<b>Architecture Target:</b> Modern OpenGL Core Profile (v4.6) &nbsp;|&nbsp; <b>OS:</b> Windows 64-bit MinGW-w64"
    )
    story.append(Paragraph(meta_footer, body_style))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 1: THE MODERN OPENGL RENDERING PHILOSOPHY
    # =======================================================================
    story.append(Paragraph("Chapter 1: The Modern OpenGL Rendering Philosophy", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph("1.1 The Paradigm Shift: Immediate Mode vs Core Profile", h2_style))
    story.append(Paragraph(
        "In vintage OpenGL (pre-3.0, known as 'Immediate Mode'), developers issued rendering commands using "
        "functions like <code>glBegin(GL_TRIANGLES)</code>, <code>glVertex3f(...)</code>, and <code>glEnd()</code>. "
        "In this old paradigm, vertex data was transmitted from the CPU to the GPU on every single draw call, "
        "creating a devastating bottleneck over the PCIe bus. Furthermore, lighting and projection relied on fixed, "
        "unmodifiable hardware matrices.",
        body_style
    ))
    story.append(Paragraph(
        "Modern OpenGL (specifically Core Profile 3.3 up through 4.6, as used in this Pong project) completely removed "
        "all fixed-function code. In Core Profile:<br/>"
        "1. <b>Zero Immediate Calls:</b> You cannot draw anything without providing your own custom GLSL shaders.<br/>"
        "2. <b>GPU-Resident Data:</b> All geometry data (vertex positions, texture coordinates) must be pre-uploaded "
        "into the GPU's high-speed video RAM (VRAM) ahead of time.<br/>"
        "3. <b>State Separation:</b> The memory holding the raw floats (VBO) is decoupled from the memory layout blueprint (VAO).",
        body_style
    ))

    story.append(Paragraph("1.2 The Programmable Graphics Pipeline Stages", h2_style))
    story.append(Paragraph(
        "Every single frame rendered in Pong passes through a deterministic series of hardware pipeline stages:",
        body_style
    ))

    pipeline_table_data = [
        ["Pipeline Stage", "Execution Unit", "Pong 2D Role / Implementation"],
        ["Vertex Fetch", "Hardware", "Reads raw vertex bytes from GPU VRAM governed by the active VAO."],
        ["Vertex Shader", "Programmable (GPU)", "Applies scale, position offsets, and transforms NDC coordinates (pong.vert / default.vert)."],
        ["Primitive Assembly", "Hardware", "Connects transformed vertices into triangles using indices from the EBO."],
        ["Rasterization", "Hardware", "Interpolates triangles across pixels on screen; determines which fragments are covered."],
        ["Fragment Shader", "Programmable (GPU)", "Calculates the final RGBA color of each pixel (solid white/gold or textured CRT noise)."],
        ["Framebuffer Tests", "Hardware", "Performs blending and writes final pixels to the double-buffered swap chain."]
    ]
    t_pipe = Table(pipeline_table_data, colWidths=[110, 110, letter[0] - 108 - 220])
    t_pipe.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#1e293b')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),
        ('FONTSIZE', (0,0), (-1,-1), 8),
        ('LEADING', (0,0), (-1,-1), 11),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor('#cbd5e1')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.HexColor('#f8fafc'), colors.white])
    ]))
    story.append(t_pipe)
    story.append(Spacer(1, 10))

    story.append(Paragraph("1.3 Normalized Device Coordinates (NDC)", h2_style))
    story.append(Paragraph(
        "Regardless of whether the game window is 800x800, 1920x1080, or 4K, modern OpenGL operates exclusively "
        "inside a normalized coordinate cube where both the X and Y axes range strictly from <b>-1.0 to +1.0</b>.<br/>"
        "• <b>Center of Screen:</b> (0.0, 0.0)<br/>"
        "• <b>Top-Left Corner:</b> (-1.0, +1.0) &nbsp;|&nbsp; <b>Bottom-Right Corner:</b> (+1.0, -1.0)<br/>"
        "In our Pong engine, the playfield is bounded between Y = +0.92 (top border) and Y = -0.92 (bottom border), "
        "leaving room for top scores and bottom keyboard tips.",
        body_style
    ))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 2: THE BUFFER TRINITY — VAO, VBO, AND EBO
    # =======================================================================
    story.append(Paragraph("Chapter 2: The Core Trinity — VAO, VBO, and EBO", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "Understanding how <b>VBO</b>, <b>VAO</b>, and <b>EBO</b> collaborate is the single most critical concept in "
        "modern OpenGL programming. Beginners often confuse them because their names sound similar, but they perform "
        "three radically distinct functions in GPU memory.",
        body_style
    ))

    # Conceptual Comparison Table
    trinity_data = [
        ["Buffer / Object", "Full OpenGL Name", "Physical Location", "Core Responsibility"],
        ["VBO", "Vertex Buffer Object", "GPU Video Memory (VRAM)", "Stores raw, unformatted binary arrays of vertex floats."],
        ["VAO", "Vertex Array Object", "GPU State Container", "Stores the 'blueprint': memory stride, offsets, and attribute pointers."],
        ["EBO (IBO)", "Element Buffer Object", "GPU Video Memory (VRAM)", "Stores index arrays telling the GPU which vertices form triangles."]
    ]
    t_trinity = Table(trinity_data, colWidths=[70, 110, 110, letter[0] - 108 - 290])
    t_trinity.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#0369a1')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),
        ('FONTSIZE', (0,0), (-1,-1), 8),
        ('LEADING', (0,0), (-1,-1), 11),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor('#cbd5e1')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.HexColor('#f0f9ff'), colors.white])
    ]))
    story.append(t_trinity)
    story.append(Spacer(1, 12))

    story.append(Paragraph("2.1 VBO (Vertex Buffer Object): The Raw Data Store", h2_style))
    story.append(Paragraph(
        "A VBO is simply a chunk of dedicated memory allocated inside the graphics card. It has no idea what its data "
        "represents—it is merely an array of contiguous bytes. In our Pong game, we have two primary VBO data structures:",
        body_style
    ))

    vbo_code = (
        "// 1. Unit Quad VBO (Used for paddles, ball, net, text)\n"
        "GLfloat quadVertices[] = {\n"
        "    0.0f, 0.0f, // Vertex 0: Bottom-Left  (X, Y)\n"
        "    1.0f, 0.0f, // Vertex 1: Bottom-Right (X, Y)\n"
        "    1.0f, 1.0f, // Vertex 2: Top-Right    (X, Y)\n"
        "    0.0f, 1.0f  // Vertex 3: Top-Left     (X, Y)\n"
        "}; // Total: 8 floats = 32 bytes\n\n"
        "// 2. Fullscreen Background Quad VBO (Used for CRT texture)\n"
        "GLfloat bgVertices[] = {\n"
        "    // Position (X, Y)   // UV Texture Coords (U, V)\n"
        "    -1.0f, -1.0f,         0.0f, 0.0f, // Vertex 0: Bottom-Left\n"
        "     1.0f, -1.0f,         1.0f, 0.0f, // Vertex 1: Bottom-Right\n"
        "     1.0f,  1.0f,         1.0f, 1.0f, // Vertex 2: Top-Right\n"
        "    -1.0f,  1.0f,         0.0f, 1.0f  // Vertex 3: Top-Left\n"
        "}; // Total: 16 floats = 64 bytes"
    )
    story.append(create_code_block(vbo_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("2.2 EBO (Element Buffer Object): Eliminating Redundant Vertices", h2_style))
    story.append(Paragraph(
        "A rectangle is formed by two adjacent triangles. If you draw two triangles without an EBO, you must provide "
        "<b>6 vertices</b> (3 per triangle). That means 2 vertices must be duplicated, wasting GPU memory and processing bandwidth.<br/>"
        "The <b>EBO (Index Buffer)</b> solves this completely: we upload only <b>4 unique vertices</b> (0, 1, 2, 3), and then "
        "upload an array of indices that tell the GPU how to connect them into two triangles:",
        body_style
    ))

    ebo_code = (
        "GLuint indices[] = {\n"
        "    0, 1, 2,   // Triangle 1: Bottom-Left -> Bottom-Right -> Top-Right\n"
        "    2, 3, 0    // Triangle 2: Top-Right   -> Top-Left     -> Bottom-Left\n"
        "};"
    )
    story.append(create_code_block(ebo_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("2.3 VAO (Vertex Array Object): The Blueprint State Container", h2_style))
    story.append(Paragraph(
        "Because a VBO is just a raw bucket of floats, the GPU cannot render it unless you tell it: "
        "<i>'Where does each vertex start? How many floats are in a position? What is the byte gap between vertices?'</i><br/>"
        "In early OpenGL, you had to call <code>glVertexAttribPointer</code> before every single draw call. "
        "The <b>VAO</b> was invented to store all of those configuration pointers inside an object. "
        "Once a VAO is configured, drawing that geometry in the render loop requires only <b>one line of code</b>: <code>VAO.Bind()</code>!",
        body_style
    ))

    callout_vao = (
        "<b>Critical OpenGL Rule — What a VAO Records:</b><br/>"
        "1. Every call to <code>glVertexAttribPointer</code> and <code>glEnableVertexAttribArray</code>.<br/>"
        "2. The specific VBO that was bound to <code>GL_ARRAY_BUFFER</code> when <code>glVertexAttribPointer</code> was called.<br/>"
        "3. The active <b>EBO</b> bound to <code>GL_ELEMENT_ARRAY_BUFFER</code> while the VAO is bound!<br/>"
        "<b>Golden Warning:</b> Never call <code>glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)</code> before unbinding your VAO, "
        "or you will unbind the EBO from the VAO itself!"
    )
    story.append(create_callout("CRITICAL ARCHITECTURAL GOTCHA", callout_vao, callout_title, callout_body, "#fffbeb", "#d97706"))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 2 CONTINUED: THE STEP-BY-STEP LIFECYCLE
    # =======================================================================
    story.append(Paragraph("2.4 Step-by-Step State Flow: How VAO, VBO, & EBO Link Together", h2_style))
    story.append(Paragraph(
        "Here is the precise sequence of operations executed during engine initialization:",
        body_style
    ))

    flow_code = (
        "// STEP 1: Generate and Bind the VAO\n"
        "// All configuration calls after this point are recorded into this VAO.\n"
        "VAO quadVAO;\n"
        "quadVAO.Bind(); // glBindVertexArray(ID)\n\n"
        "// STEP 2: Generate VBO and Upload Vertex Data to GPU VRAM\n"
        "VBO quadVBO(quadVertices, sizeof(quadVertices)); // glGenBuffers, glBindBuffer, glBufferData\n\n"
        "// STEP 3: Generate EBO and Upload Index Data to GPU VRAM\n"
        "// Because quadVAO is active, the EBO is automatically registered inside quadVAO!\n"
        "EBO quadEBO(quadIndices, sizeof(quadIndices)); // glGenBuffers, glBindBuffer, glBufferData\n\n"
        "// STEP 4: Define the Memory Layout Blueprint\n"
        "// Tell the VAO: layout=0, 2 components (X, Y), floats, stride=2*sizeof(float), offset=0\n"
        "quadVAO.LinkAttrib(quadVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);\n\n"
        "// STEP 5: Unbind to prevent accidental state modification\n"
        "quadVAO.Unbind(); // glBindVertexArray(0)\n"
        "quadVBO.Unbind(); // glBindBuffer(GL_ARRAY_BUFFER, 0)\n"
        "quadEBO.Unbind(); // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)\n\n"
        "// ============================================================\n"
        "// IN THE GAME LOOP (Every Frame):\n"
        "// ============================================================\n"
        "pongShader.Activate();\n"
        "quadVAO.Bind(); // Instantly restores VBO attributes AND the bound EBO!\n"
        "glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draws 2 triangles\n"
        "quadVAO.Unbind();"
    )
    story.append(create_code_block(flow_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("2.5 Stride and Offset Mathematics in Vertex Attributes", h2_style))
    story.append(Paragraph(
        "The call to <code>glVertexAttribPointer(layout, size, type, normalized, stride, pointer)</code> contains "
        "two crucial arguments that frequently confuse graphics programmers:",
        body_style
    ))

    stride_table = [
        ["Attribute Parameter", "Meaning in Pong 2D", "Quad VBO Value", "Background VBO Value"],
        ["layout (index)", "Matches layout(location = X) in shader", "0 (aPos)", "0 (aPos) & 1 (aTex)"],
        ["numComponents", "Number of components per attribute", "2 (X, Y)", "2 for Pos, 2 for UV"],
        ["type", "Data type of each component", "GL_FLOAT (4 bytes)", "GL_FLOAT (4 bytes)"],
        ["stride", "Byte gap between successive vertices", "2 * 4 = 8 bytes", "4 * 4 = 16 bytes"],
        ["offset (pointer)", "Byte start offset of attribute in vertex", "(void*)0", "(void*)0 (Pos), (void*)8 (UV)"]
    ]
    t_stride = Table(stride_table, colWidths=[95, 160, 105, letter[0] - 108 - 360])
    t_stride.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#0f172a')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),
        ('FONTSIZE', (0,0), (-1,-1), 8),
        ('LEADING', (0,0), (-1,-1), 11),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor('#cbd5e1')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.HexColor('#f8fafc'), colors.white])
    ]))
    story.append(t_stride)
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 3: CUSTOM ENGINE ABSTRACTION CLASSES
    # =======================================================================
    story.append(Paragraph("Chapter 3: Custom Engine Abstraction Classes", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "To prevent OpenGL code from turning into an unmaintainable tangle of raw integer IDs and boilerplate calls, "
        "this project wraps OpenGL primitives into clean, object-oriented C++ classes.",
        body_style
    ))

    story.append(Paragraph("3.1 The VBO Class (VBO.h / VBO.cpp)", h2_style))
    story.append(Paragraph(
        "The <code>VBO</code> class encapsulates the creation, binding, and deletion of a GPU vertex buffer:",
        body_style
    ))
    vbo_class_code = (
        "VBO::VBO(GLfloat* vertices, GLsizeiptr size) {\n"
        "    glGenBuffers(1, &ID); // Generate unique GPU buffer ID\n"
        "    glBindBuffer(GL_ARRAY_BUFFER, ID); // Bind target\n"
        "    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); // Copy data from CPU RAM to GPU VRAM\n"
        "}\n\n"
        "void VBO::Bind()   { glBindBuffer(GL_ARRAY_BUFFER, ID); }\n"
        "void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }\n"
        "void VBO::Delete() { glDeleteBuffers(1, &ID); }"
    )
    story.append(create_code_block(vbo_class_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("3.2 The VAO Class (VAO.h / VAO.cpp)", h2_style))
    story.append(Paragraph(
        "The <code>VAO</code> class acts as the conductor, providing the <code>LinkAttrib</code> method that bridges "
        "a VBO to a specific shader attribute location:",
        body_style
    ))
    vao_class_code = (
        "void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {\n"
        "    VBO.Bind();\n"
        "    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);\n"
        "    glEnableVertexAttribArray(layout);\n"
        "    VBO.Unbind();\n"
        "}"
    )
    story.append(create_code_block(vao_class_code, code_style))
    story.append(PageBreak())

    story.append(Paragraph("3.3 The Shader Class (shaderClass.h / ShaderClass.cpp)", h2_style))
    story.append(Paragraph(
        "The <code>Shader</code> class handles reading GLSL text files from disk, compiling the Vertex and Fragment "
        "shaders independently on the GPU, linking them into a unified GPU program, and inspecting the compilation status:",
        body_style
    ))
    shader_class_code = (
        "// Inside Shader::compileErrors:\n"
        "glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);\n"
        "if (hasCompiled == GL_FALSE) {\n"
        "    glGetShaderInfoLog(shader, 1024, NULL, infoLog);\n"
        "    std::cout << \"SHADER_COMPILATION_ERROR for: \" << type << \"\\n\" << infoLog << \"\\n\";\n"
        "}"
    )
    story.append(create_code_block(shader_class_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("3.4 The Texture Class (Texture.h / Texture.cpp)", h2_style))
    story.append(Paragraph(
        "The <code>Texture</code> class leverages Sean Barrett's industry-standard <code>stb_image</code> library "
        "to load raw PNG image bytes from disk, flip image coordinates to match OpenGL's bottom-left origin, "
        "and configure texture sampling parameters:",
        body_style
    ))
    tex_code = (
        "stbi_set_flip_vertically_on_load(true);\n"
        "unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);\n"
        "glGenTextures(1, &ID);\n"
        "glActiveTexture(slot); // e.g., GL_TEXTURE0\n"
        "glBindTexture(texType, ID);\n"
        "glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);\n"
        "glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Crisp pixel art\n"
        "glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);\n"
        "glGenerateMipmap(texType);\n"
        "stbi_image_free(bytes);"
    )
    story.append(create_code_block(tex_code, code_style))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 4: SHADERS & THE UNIT QUAD ARCHITECTURE
    # =======================================================================
    story.append(Paragraph("Chapter 4: Shaders & The Unit Quad Architecture", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "A common beginner mistake in 2D game development is re-uploading new vertex coordinates to the GPU whenever "
        "a paddle moves or the ball bounces. In this project, we employ an elite technique: <b>The Unit Quad Architecture</b>.",
        body_style
    ))

    story.append(Paragraph("4.1 The Unit Quad Transform Mathematics", h2_style))
    story.append(Paragraph(
        "We upload exactly <b>one</b> square quad with local coordinates <code>[0.0, 1.0] x [0.0, 1.0]</code> to the GPU at startup. "
        "Inside <code>pong.vert</code>, the vertex shader dynamically stretches and moves this unit quad using two uniform vectors: "
        "<code>scale</code> and <code>offset</code>:",
        body_style
    ))

    pong_vert_code = (
        "#version 460 core\n"
        "layout (location = 0) in vec2 aPos; // Raw unit quad: (0,0), (1,0), (1,1), (0,1)\n\n"
        "uniform vec2 offset; // Position on screen (NDC coordinates)\n"
        "uniform vec2 scale;  // Width and Height in NDC units\n\n"
        "void main() {\n"
        "    // Formula: TransformedPos = (UnitPos * Scale) + Offset\n"
        "    gl_Position = vec4(aPos.x * scale.x + offset.x, aPos.y * scale.y + offset.y, 0.0, 1.0);\n"
        "}"
    )
    story.append(create_code_block(pong_vert_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("4.2 How Everything is Drawn with One Single Buffer", h2_style))
    story.append(Paragraph(
        "Because of this mathematical transformation in the shader, the CPU never has to modify GPU vertex memory! "
        "Look at how various game elements are drawn by simply changing uniforms:",
        body_style
    ))

    quad_usage_data = [
        ["Game Element", "Scale Uniform (Width, Height)", "Offset Uniform (X, Y)", "Color Uniform (R, G, B)"],
        ["Top Border", "scale = (2.0f, 0.02f)", "offset = (-1.0f, +0.92f)", "White (1.0, 1.0, 1.0)"],
        ["Net Dash", "scale = (0.012f, 0.045f)", "offset = (-0.006f, y_pos)", "White (1.0, 1.0, 1.0)"],
        ["Player 1 Paddle", "scale = (0.032f, 0.25f)", "offset = (-0.92f, p1_y - 0.125f)", "White (1.0, 1.0, 1.0)"],
        ["Square Ball", "scale = (0.028f, 0.028f)", "offset = (ball_x - 0.014f, ball_y - 0.014f)", "White (1.0, 1.0, 1.0)"],
        ["Font Pixel", "scale = (pixelW*0.95, pixelH*0.95)", "offset = (startX, py)", "Ice Cyan (0.2, 0.9, 1.0)"]
    ]
    t_quad = Table(quad_usage_data, colWidths=[90, 130, 130, letter[0] - 108 - 350])
    t_quad.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#1e293b')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),
        ('FONTSIZE', (0,0), (-1,-1), 8),
        ('LEADING', (0,0), (-1,-1), 11),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor('#cbd5e1')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.HexColor('#f8fafc'), colors.white])
    ]))
    story.append(t_quad)
    story.append(Spacer(1, 10))

    story.append(Paragraph("4.3 The Fragment Shaders: Solid Color vs CRT Noise", h2_style))
    story.append(Paragraph(
        "• <b><code>pong.frag</code>:</b> Uses a simple <code>uniform vec3 color;</code> to paint pixels solid white, gold, or ice-cyan.<br/>"
        "• <b><code>default.frag</code>:</b> Uses <code>uniform sampler2D tex0;</code> and texture coordinates from <code>default.vert</code> "
        "to sample <code>background.png</code>, giving the screen its authentic 1970s CRT grain look.",
        body_style
    ))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 5: THE 5x3 RETRO BITMAP FONT ENGINE
    # =======================================================================
    story.append(Paragraph("Chapter 5: The 5x3 Retro Bitmap Font Engine", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "Most OpenGL games rely on heavy external libraries like FreeType or font texture atlases to display text. "
        "In this project, we implemented a custom <b>Bitwise Bitmap Font Generator</b> right inside C++ without any third-party dependencies.",
        body_style
    ))

    story.append(Paragraph("5.1 Bitwise Glyph Representation", h2_style))
    story.append(Paragraph(
        "Every character in our font is 5 rows high by 3 columns wide. A 3-column row requires only <b>3 bits</b> of data! "
        "Therefore, an entire 5x3 glyph is represented as an array of 5 unsigned 8-bit integers (<code>uint8_t</code>):<br/>"
        "• Bit 2 (0b100) = Left column<br/>"
        "• Bit 1 (0b010) = Middle column<br/>"
        "• Bit 0 (0b001) = Right column",
        body_style
    ))

    font_code = (
        "// Example: The digit '0' (5 rows high, 3 columns wide)\n"
        "static const uint8_t FONT_0[5] = {\n"
        "    0b111,  // Row 0: ■ ■ ■  (Top bar)\n"
        "    0b101,  // Row 1: ■   ■  (Left & Right walls)\n"
        "    0b101,  // Row 2: ■   ■  (Left & Right walls)\n"
        "    0b101,  // Row 3: ■   ■  (Left & Right walls)\n"
        "    0b111   // Row 4: ■ ■ ■  (Bottom bar)\n"
        "};\n\n"
        "// Example: The letter 'P' (Used in 'PAUSED')\n"
        "static const uint8_t FONT_P[5] = {\n"
        "    0b111,  // Row 0: ■ ■ ■\n"
        "    0b101,  // Row 1: ■   ■\n"
        "    0b111,  // Row 2: ■ ■ ■\n"
        "    0b100,  // Row 3: ■\n"
        "    0b100   // Row 4: ■\n"
        "};"
    )
    story.append(create_code_block(font_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("5.2 Decoding Bits into OpenGL Quads", h2_style))
    story.append(Paragraph(
        "Inside <code>RenderChar(...)</code>, the engine loops over each row from 0 to 4, and uses the bitwise AND operator "
        "(<code>&</code>) to test whether a pixel should be drawn:",
        body_style
    ))

    render_char_code = (
        "void RenderChar(char c, float startX, float startY, float pixelW, float pixelH, GLuint offsetLoc, GLuint scaleLoc) {\n"
        "    const uint8_t* rows = GetCharBitmap(c);\n"
        "    glUniform2f(scaleLoc, pixelW * 0.95f, pixelH * 0.95f);\n\n"
        "    for (int r = 0; r < 5; r++) {\n"
        "        uint8_t rowVal = rows[r];\n"
        "        float py = startY + (4 - r) * pixelH; // Invert row so row 0 is at top\n\n"
        "        if (rowVal & 0b100) { // Test Left Column\n"
        "            glUniform2f(offsetLoc, startX, py);\n"
        "            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);\n"
        "        }\n"
        "        if (rowVal & 0b010) { // Test Middle Column\n"
        "            glUniform2f(offsetLoc, startX + pixelW, py);\n"
        "            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);\n"
        "        }\n"
        "        if (rowVal & 0b001) { // Test Right Column\n"
        "            glUniform2f(offsetLoc, startX + 2.0f * pixelW, py);\n"
        "            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);\n"
        "        }\n"
        "    }\n"
        "}"
    )
    story.append(create_code_block(render_char_code, code_style))
    story.append(Spacer(1, 8))
    story.append(Paragraph(
        "By setting <code>pixelW</code> and <code>pixelH</code>, the exact same function draws the massive scoreboard numbers "
        "(0.022x0.026), the medium win counters (0.010x0.012), and the small controls help text (0.007x0.009)!",
        body_style
    ))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 6: GAME PHYSICS, COLLISIONS, AND MATH
    # =======================================================================
    story.append(Paragraph("Chapter 6: Game Physics, Collisions, and Math", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "Pong's classic feel depends entirely on its ball bounce deflection mathematics and progressive rally acceleration.",
        body_style
    ))

    story.append(Paragraph("6.1 Axis-Aligned Bounding Box (AABB) Collision Detection", h2_style))
    story.append(Paragraph(
        "To check if the ball hits a paddle, we perform an AABB overlap test in 2D space. For Player 1 (Left paddle):<br/>"
        "• Ball X bounds: <code>[ball_x - halfBall, ball_x + halfBall]</code><br/>"
        "• Paddle X bounds: <code>[p1_x, p1_x + PADDLE_WIDTH]</code><br/>"
        "• Ball Y bounds: <code>[ball_y - halfBall, ball_y + halfBall]</code><br/>"
        "• Paddle Y bounds: <code>[p1_y - halfPaddleH, p1_y + halfPaddleH]</code><br/>"
        "Additionally, we verify that <code>ball_vx &lt; 0.0f</code> so that collision triggers only when the ball is traveling <i>toward</i> the paddle.",
        body_style
    ))

    story.append(Paragraph("6.2 Dynamic Trigonometric Angle Deflection", h2_style))
    story.append(Paragraph(
        "If a paddle behaved like a flat wall, the ball would simply bounce off at the same angle it entered, making the game boring. "
        "In our engine, the reflection angle depends entirely on <b>where on the paddle</b> the ball strikes:",
        body_style
    ))

    phys_code = (
        "// 1. Calculate relative hit position normalized from -1.0 (bottom edge) to +1.0 (top edge)\n"
        "float relativeHit = (ball_y - p1_y) / halfPaddleH;\n"
        "relativeHit = std::clamp(relativeHit, -1.0f, 1.0f);\n\n"
        "// 2. Map relative hit to a deflection angle up to +/- 55 degrees\n"
        "float bounceAngle = relativeHit * (55.0f * 3.14159265f / 180.0f);\n\n"
        "// 3. Accelerate ball velocity (1.05x speed per paddle strike)\n"
        "currentBallSpeed = std::min(currentBallSpeed * BALL_SPEED_INCREMENT, BALL_MAX_SPEED);\n\n"
        "// 4. Resolve new velocity components using trigonometry\n"
        "ball_vx = currentBallSpeed * cos(bounceAngle); // Positive X (toward right)\n"
        "ball_vy = currentBallSpeed * sin(bounceAngle);"
    )
    story.append(create_code_block(phys_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("6.3 Progressive Rally Acceleration & Wall Bounces", h2_style))
    story.append(Paragraph(
        "• <b>Speed Multiplier:</b> Every time a paddle successfully hits the ball, <code>currentBallSpeed</code> multiplies by "
        "<code>1.05f</code> (5% faster). This creates tense, fast-paced rallies that max out at <code>BALL_MAX_SPEED = 2.4f</code>.<br/>"
        "• <b>Wall Inversion:</b> When striking the top (Y = +0.92) or bottom (Y = -0.92) boundary, the Y velocity component is inverted "
        "(<code>ball_vy = -abs(ball_vy)</code>), and the ball's position is clamped to prevent it from escaping the arena.",
        body_style
    ))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 7: GAME LOOP, TIMING & SYSTEMS ARCHITECTURE
    # =======================================================================
    story.append(Paragraph("Chapter 7: Game Loop, Timing & Systems Architecture", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph("7.1 Delta Time & The Anti-Tunneling Clamp", h2_style))
    story.append(Paragraph(
        "Games must run at the exact same physical speed regardless of whether a player's monitor runs at 60Hz, 144Hz, or 240Hz. "
        "We calculate <code>deltaTime = currentFrameTime - lastFrameTime</code> on every tick.<br/>"
        "However, if a user drags the window or a lag spike occurs, <code>deltaTime</code> could jump to 0.5s, which would cause the ball "
        "to 'tunnel' right through the paddle! We clamp delta time to protect against tunneling:",
        body_style
    ))

    dt_code = (
        "float currentFrameTime = (float)glfwGetTime();\n"
        "float deltaTime = currentFrameTime - lastFrameTime;\n"
        "lastFrameTime = currentFrameTime;\n"
        "if (deltaTime > 0.05f) deltaTime = 0.05f; // Clamped to 50ms max step"
    )
    story.append(create_code_block(dt_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("7.2 Edge-Triggered Input & The 'P' Freeze Key", h2_style))
    story.append(Paragraph(
        "In GLFW, <code>glfwGetKey(window, GLFW_KEY_P)</code> returns <code>GLFW_PRESS</code> on every single frame that the key is held down. "
        "If you simply wrote <code>if (pressed) isPaused = !isPaused;</code>, the game would toggle pause 144 times a second!<br/>"
        "We implemented <b>Edge-Triggered Debouncing</b>:",
        body_style
    ))

    pause_code = (
        "bool pKeyPressed = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);\n"
        "if (pKeyPressed && !pKeyWasPressed) {\n"
        "    isPaused = !isPaused; // Only fires once on the rising edge!\n"
        "}\n"
        "pKeyWasPressed = pKeyPressed;\n\n"
        "if (!matchOver && !isPaused) {\n"
        "    // Only update paddles and ball physics when NOT paused!\n"
        "}"
    )
    story.append(create_code_block(pause_code, code_style))
    story.append(Spacer(1, 10))

    story.append(Paragraph("7.3 Asynchronous Audio Synthesis via Windows Beep", h2_style))
    story.append(Paragraph(
        "The Win32 API provides <code>Beep(DWORD dwFreq, DWORD dwDuration)</code>. However, <code>Beep()</code> is a <b>synchronous blocking call</b>—"
        "if you call <code>Beep(540, 100)</code> on the main thread, the entire game will freeze for 100 milliseconds!<br/>"
        "We solved this by spawning detached C++ worker threads:",
        body_style
    ))

    audio_code = (
        "void PlaySoundAsync(int freq, int durationMs) {\n"
        "#ifdef _WIN32\n"
        "    std::thread([=]() {\n"
        "        Beep(freq, durationMs); // Executes on background thread\n"
        "    }).detach(); // Thread terminates automatically\n"
        "#endif\n"
        "}"
    )
    story.append(create_code_block(audio_code, code_style))
    story.append(PageBreak())

    # =======================================================================
    # CHAPTER 8: COMPILATION & TOOLCHAIN BREAKDOWN
    # =======================================================================
    story.append(Paragraph("Chapter 8: Compilation & Toolchain Breakdown", h1_style))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor('#cbd5e1'), spaceAfter=12))

    story.append(Paragraph(
        "The compilation command inside <code>build.ps1</code> turns raw C++ code into a high-performance Windows binary:",
        body_style
    ))

    flags_data = [
        ["Compiler Flag", "Technical Purpose in Pong 2D"],
        ["-std=c++20", "Enables modern C++ features: lambdas, auto types, std::clamp, and standard concurrency."],
        ["-mwindows", "Targets the Windows GUI subsystem; suppresses the black command prompt window."],
        ["-static", "Statically links C/C++ runtimes into the .exe so it runs on PCs without MinGW installed."],
        ["-static-libgcc", "Embeds the GCC runtime library directly inside the binary."],
        ["-static-libstdc++", "Embeds the C++ Standard Template Library directly inside the binary."],
        ["-Isource/engine", "Tells the preprocessor where to search for header files (#include)."],
        ["-lglfw3", "Links the static GLFW library (window management, input, OpenGL context)."],
        ["-lopengl32", "Links Microsoft Windows' core OpenGL driver interface."],
        ["-lgdi32", "Links the Windows Graphics Device Interface (required for GLFW window creation)."]
    ]
    t_flags = Table(flags_data, colWidths=[130, letter[0] - 108 - 130])
    t_flags.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#1e293b')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),
        ('FONTSIZE', (0,0), (-1,-1), 8),
        ('LEADING', (0,0), (-1,-1), 11),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor('#cbd5e1')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.HexColor('#f8fafc'), colors.white])
    ]))
    story.append(t_flags)
    story.append(Spacer(1, 15))

    final_callout = (
        "<b>Summary & Architectural Checklist:</b><br/>"
        "• <b>VBO:</b> Holds the raw unit quad vertices on the GPU.<br/>"
        "• <b>VAO:</b> Remembers vertex attribute formatting and links to the VBO & EBO.<br/>"
        "• <b>EBO:</b> Instructs the GPU to draw two triangles using 4 indices.<br/>"
        "• <b>Shaders:</b> Scales and offsets the unit quad dynamically per draw call.<br/>"
        "• <b>Bitmap Font:</b> Draws text pixel-by-pixel using bitwise masks and the unit quad.<br/>"
        "• <b>Threads:</b> Plays retro beeps asynchronously without stalling the rendering loop."
    )
    story.append(create_callout("ARCHITECTURAL EXCELLENCE", final_callout, callout_title, callout_body, "#ecfdf5", "#059669"))

    # Build Document
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"Successfully generated book: {pdf_filename}")

if __name__ == '__main__':
    build_book()
