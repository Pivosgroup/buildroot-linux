################################################################################
# GENDOC -- generates the make targets needed to build a specific type of
#           asciidoc documentation.
#
#  argument 1 is the name of the document and must be a subdirectory of docs/;
#             the top-level asciidoc file must have the same name
#  argument 2 is the type of document to generate (-f argument of a2x)
#  argument 3 is the document type as used in the make target
#  argument 4 is the output file extension for the document type
#  argument 5 is the human text for the document type
#  argument 6 (optional) are extra arguments for a2x
#
# The variable <DOCUMENT_NAME>_SOURCES defines the dependencies.
################################################################################
define GENDOC_INNER
$(1): $(1)-$(3)
.PHONY: $(1)-$(3)
$(1)-$(3): $$(O)/docs/$(1)/$(1).$(4)

$$(O)/docs/$(1)/$(1).$(4): docs/$(1)/$(1).txt $$($(call UPPERCASE,$(1))_SOURCES)
	@echo "Generating $(5) $(1)..."
	$(Q)mkdir -p $$(@D)
	$(Q)a2x $(6) -f $(2) -d book -L -r $(TOPDIR)/docs/images \
	  -D $$(@D) $$<
endef

################################################################################
# GENDOC -- generates the make targets needed to build asciidoc documentation.
#
#  argument 1 is the name of the document and must be a subdirectory of docs/;
#             the top-level asciidoc file must have the same name
#
# The variable <DOCUMENT_NAME>_SOURCES defines the dependencies.
################################################################################
define GENDOC
$(call GENDOC_INNER,$(1),xhtml,html,html,HTML,--xsltproc-opts "--stringparam toc.section.depth 4")
$(call GENDOC_INNER,$(1),chunked,split-html,chunked,Split HTML,--xsltproc-opts "--stringparam toc.section.depth 4")
$(call GENDOC_INNER,$(1),pdf,pdf,pdf,PDF,--dblatex-opts "-P latex.output.revhistory=0")
$(call GENDOC_INNER,$(1),text,txt,text,Text)
$(call GENDOC_INNER,$(1),epub,epub,epub,EPUB)
clean: $(1)-clean
$(1)-clean:
	$(Q)$(RM) -rf $(O)/docs/$(1)
.PHONY: $(1) $(1)-clean
endef

MANUAL_SOURCES = $(wildcard docs/manual/*.txt) $(wildcard docs/images/*)
$(eval $(call GENDOC,manual))
