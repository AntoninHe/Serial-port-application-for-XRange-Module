/*******************************************************************************
 *
 * Copyright (c) 2015 Thomas Telkamp
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 *******************************************************************************/

#ifndef FORWADER_HPP
#define FORWADER_HPP

void parseCommandline(int argc, char *argv[]);

void forwarder(char my_msg[], int receivedbytes);

#endif
