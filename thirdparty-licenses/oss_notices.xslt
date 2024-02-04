<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" />
    <xsl:template match="/">
        <xsl:for-each select="oss/software">
            <xsl:value-of select="name" />
            <xsl:text disable-output-escaping="yes">
</xsl:text>
            <xsl:value-of select="license_text" />
            <xsl:text disable-output-escaping="yes">  

===============================================================================

</xsl:text>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>