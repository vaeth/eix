<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="xml" indent="yes"
		doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
		doctype-public="-//W3C//DTD XHTML 1.1//EN" />

	<xsl:template match="cache">
		<html>
			<xsl:apply-templates/>
		</html>
	</xsl:template>

	<xsl:template match="body">
		<body>
			<xsl:apply-templates/>
		</body>
	</xsl:template>

	<xsl:template match="head">
		<head>
			<link href="{@style}" rel="stylesheet" type="text/css"/>
			<title><xsl:apply-templates/></title>
		</head>
	</xsl:template>

	<xsl:template match="current">
		<xsl:value-of select="/cache/@current"/>
	</xsl:template>

	<xsl:template match="section">
		<div class="section">
			<h3><xsl:value-of select="@title"/></h3>
			<xsl:apply-templates/>
		</div>
	</xsl:template>

	<xsl:template name="splittype">
		<xsl:param name="str"/>
		<xsl:choose>
			<xsl:when test="contains($str,'/')">
				<a href="#{substring-before($str,'/')}">
					<xsl:value-of select="substring-before($str,'/')"/></a>&lt;<xsl:call-template name="splittype">
					<xsl:with-param name="str" select="substring-after($str,'/')" />
				</xsl:call-template>&gt; 
			</xsl:when>
			<xsl:otherwise>
				<a href="#{$str}"><xsl:value-of select="$str"/></a>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="format">
		<ol class="format">
			<xsl:apply-templates/>
		</ol>
	</xsl:template>

	<xsl:template match="field">
		<li class="field">
			<span class="name"><xsl:value-of select="@name"/></span>
			(<span class="type"><xsl:call-template name="splittype">
					<xsl:with-param name="str" select="@type"/>
			</xsl:call-template></span>)
			<xsl:apply-templates/>
		</li>
	</xsl:template>

	<xsl:template match="enumeration">
		<ul class="enumeration">
			<xsl:apply-templates/>
		</ul>
	</xsl:template>

	<xsl:template match="enum">
		<li class="enum"><xsl:value-of select="@value"/> - <xsl:value-of select="@name"/></li>
	</xsl:template>

	<xsl:template match="bitmask">
		<ul class="bitmask">
			<xsl:apply-templates/>
		</ul>
	</xsl:template>

	<xsl:template match="mask">
		<li><xsl:value-of select="@value"/> - <xsl:value-of select="@name"/></li>
	</xsl:template>

	<xsl:template match="block">
		<div class="block" id="{@name}">
			<h3><xsl:value-of select="@name"/></h3>
			<xsl:apply-templates/>
		</div>
	</xsl:template>

	<xsl:template match="foot">
		<hr/>
		<div id="foot">
			<a href="http://validator.w3.org/check?uri=referer">Valid XHTML 1.1</a> - 
			<a href="http://jigsaw.w3.org/css-validator/check/referer">Valid CSS!</a><br/>
			<xsl:apply-templates/>
		</div>
	</xsl:template>

	<!-- ## Basic HTML-Tags ## -->

	<xsl:template match="p">
		<p><xsl:apply-templates/></p>
	</xsl:template>

	<xsl:template match="h3">
		<h3><xsl:apply-templates/></h3>
	</xsl:template>

	<xsl:template match="h2">
		<h2><xsl:apply-templates/></h2>
	</xsl:template>

	<xsl:template match="strong">
		<strong><xsl:apply-templates/></strong>
	</xsl:template>

	<xsl:template match="br">
		<br><xsl:apply-templates/></br>
	</xsl:template>
		
	<xsl:template match="nobr">
		<span style="white-space: nowrap;"><xsl:apply-templates/></span>
	</xsl:template>


</xsl:stylesheet>
