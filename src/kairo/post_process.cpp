void PostProcess(EngineContext& engineContext) {
 //create framebuffer
    engineContext.postProcessingFB = 0;
    glGenFramebuffers(1, &engineContext.postProcessingFB);
    glBindFramebuffer(GL_FRAMEBUFFER, engineContext.postProcessingFB);

    //create color texture
    engineContext.texColorBuffer = 0;
    glGenTextures(1, &engineContext.texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, engineContext.texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(engineContext.scrWidth), static_cast<int>(engineContext.scrHeight), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    //attach texture to currently bound framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, engineContext.texColorBuffer, 0);

    //create renderbuffer object for depth and stencil attachment
    engineContext.rboDepthStencil = 0;
    glGenRenderbuffers(1, &engineContext.rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, engineContext.rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(engineContext.scrWidth), static_cast<int>(engineContext.scrHeight));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //attach renderbuffer to framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, engineContext.rboDepthStencil);

    //check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float PPQuadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    engineContext.PPVBO = 0;
    glGenBuffers(1, &engineContext.PPVBO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.PPVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PPQuadVertices), PPQuadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    engineContext.PPVAO = 0;
    glGenVertexArrays(1, &engineContext.PPVAO);
    glBindVertexArray(engineContext.PPVAO);
    glBindBuffer(GL_ARRAY_BUFFER, engineContext.PPVBO);

    // set up vertex attributes
        // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        // texcoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}