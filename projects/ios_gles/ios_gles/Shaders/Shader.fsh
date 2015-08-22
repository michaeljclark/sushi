//
//  Shader.fsh
//  ios_gles
//
//  Created by Michael Clark on 23/8/15.
//  Copyright (c) 2015 Michael Clark. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
