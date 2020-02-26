/** @file splat_run.cpp
 *
 * File created by Peter Watkins (KE7IST) 6/17/15.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <bzlib.h>
#include <vector>
#include <string>
#include "site.h"
#include "path.h"
#include "dem.h"
#include "itwom3.0.h"
#include "splat_run.h"

using namespace std;

const std::string SplatRun::splat_name = "SPLAT!";
const std::string SplatRun::splat_version = "1.4.2";
