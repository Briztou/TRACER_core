
#
# This file is a template makefile that contains all sections and their conditionals in the correct order to implement
#	proc build makefiles;
#

#--------------------------------------------------- local env config --------------------------------------------------



#--------------------------------------------------- build env config --------------------------------------------------



#--------------------------------------------------- khal env config ---------------------------------------------------

ifdef BUILD_KHAL

endif


#------------------------------------------------- arch mods env config ------------------------------------------------

ifdef BUILD_ARCH_MODS

endif


#------------------------------------------------------ proc family -----------------------------------------------------

include proc_family.mk


#--------------------------------------------------- default options ---------------------------------------------------

include default_options.mk


#------------------------------------------------------ khal rules -----------------------------------------------------

ifdef BUILD_KHAL

endif


#--------------------------------------------------- arch_mods rules ---------------------------------------------------

ifdef BUILD_ARCH_MODS

endif