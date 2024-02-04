<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" />
    <xsl:template match="/">
    <xsl:text disable-output-escaping="yes">
# List of Licenses
</xsl:text>
        <xsl:for-each select="oss/software">
            <xsl:text disable-output-escaping="yes">
**Open-Source Software:** </xsl:text>
            <xsl:value-of select="name" />
            <xsl:text disable-output-escaping="yes">  
**Version of the OSS:** </xsl:text>
            <xsl:value-of select="version" />
            <xsl:text disable-output-escaping="yes">  
**Redistribution Format:** </xsl:text>
            <xsl:value-of select="format" />
            <xsl:text disable-output-escaping="yes">  
**Any Modification to the OSS?** </xsl:text>
            <xsl:value-of select="modified" />
            <xsl:text disable-output-escaping="yes">  
**URL of the OSS:** </xsl:text>
            <xsl:value-of select="url" />
            <xsl:text disable-output-escaping="yes">  
**Licensing Information URL:** </xsl:text>
            <xsl:value-of select="readme_url" />
            <xsl:text disable-output-escaping="yes">  
**Licensing Information Included in the OSS:** </xsl:text>
            <xsl:value-of select="license_url" />
            <xsl:text disable-output-escaping="yes">  
**License Identifier:** </xsl:text>
            <xsl:value-of select="spdx_identifier" />
            <xsl:text disable-output-escaping="yes">  

---
</xsl:text>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>