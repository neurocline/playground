import os
import sys
sys.path.append(os.path.abspath("libs"))
print(sys.path)

# this is the outer main
from inner import *

main()
