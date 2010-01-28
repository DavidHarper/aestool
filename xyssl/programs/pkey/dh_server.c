/*
 *  Diffie-Hellman-Merkle key exchange (server side)
 *
 *  Copyright (C) 2006-2007  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdio.h>

#include "xyssl/net.h"
#include "xyssl/aes.h"
#include "xyssl/dhm.h"
#include "xyssl/rsa.h"
#include "xyssl/sha1.h"
#include "xyssl/havege.h"

#define SERVER_PORT 11999
#define PLAINTEXT "0123456_89ABCDE_"

int main( void )
{
    FILE *f;

    int ret, n, buflen;
    int listen_fd = -1;
    int client_fd = -1;

    unsigned char buf[1024];
    unsigned char hash[20];
    unsigned char buf2[2];

    havege_state hs;
    rsa_context rsa;
    dhm_context dhm;
    aes_context aes;

    memset( &rsa, 0, sizeof( rsa ) );
    memset( &dhm, 0, sizeof( dhm ) );

    /*
     * 1. Setup the RNG
     */
    printf( "\n  . Seeding the random number generator" );
    fflush( stdout );

    havege_init( &hs );

    /*
     * 2a. Read the server's private RSA key
     */
    printf( "\n  . Reading private key from rsa_priv.txt" );
    fflush( stdout );

    if( ( f = fopen( "rsa_priv.txt", "rb" ) ) == NULL )
    {
        ret = 1;
        printf( " failed\n  ! Could not open rsa_priv.txt\n" \
                "  ! Please run rsa_genkey first\n\n" );
        goto exit;
    }

    if( ( ret = rsa_read_private( &rsa, f ) ) != 0 )
    {
        printf( " failed\n  ! rsa_read_private returned %08x\n\n", ret );
        goto exit;
    }

    fclose( f );

    /*
     * 2b. Get the DHM modulus and generator
     */
    printf( "\n  . Reading DH parameters from dh_prime.txt" );
    fflush( stdout );

    if( ( f = fopen( "dh_prime.txt", "rb" ) ) == NULL )
    {
        ret = 1;
        printf( " failed\n  ! Could not open dh_prime.txt\n" \
                "  ! Please run dh_genprime first\n\n" );
        goto exit;
    }

    if( mpi_read_file( &dhm.P, 16, f ) != 0 ||
        mpi_read_file( &dhm.G, 16, f ) != 0 )
    {
        printf( " failed\n  ! Invalid DH parameter file\n\n" );
        goto exit;
    }

    /*
     * 3. Wait for a client to connect
     */
    printf( "\n  . Waiting for a remote connection" );
    fflush( stdout );

    if( ( ret = net_bind( &listen_fd, NULL, SERVER_PORT ) ) != 0 )
    {
        printf( " failed\n  ! net_bind returned %08x\n\n", ret );
        goto exit;
    }

    if( ( ret = net_accept( listen_fd, &client_fd, NULL ) ) != 0 )
    {
        printf( " failed\n  ! net_accept returned %08x\n\n", ret );
        goto exit;
    }

    /*
     * 4. Setup the DH parameters (P,G,Ys)
     */
    printf( "\n  . Sending the server's DH parameters" );
    fflush( stdout );

    memset( buf, 0, sizeof( buf ) );

    if( ( ret = dhm_make_params( &dhm, havege_rand, &hs,
                                 buf, &n ) ) != 0 )
    {
        printf( " failed\n  ! dhm_make_params returned %08x\n\n", ret );
        goto exit;
    }

    /*
     * 5. Sign the parameters and send them
     */
    sha1( buf, n, hash );

    buf[n] = rsa.len >> 8;
    buf[n + 1] = rsa.len;

    if( ( ret = rsa_pkcs1_sign( &rsa, RSA_SHA1, hash, 20,
                                buf + n + 2, rsa.len ) ) != 0 )
    {
        printf( " failed\n  ! rsa_pkcs1_sign returned %08x\n\n", ret );
        goto exit;
    }

    buflen = n + 2 + rsa.len;
    buf2[0] = buflen >> 8;
    buf2[1] = buflen;
    n = 2;

    if( ( ret = net_send( client_fd, buf2, &n ) ) != 0 )
    {
        printf( " failed\n  ! net_send returned %08x\n\n", ret );
        goto exit;
    }

    if( ( ret = net_send( client_fd, buf, &buflen ) ) != 0 )
    {
        printf( " failed\n  ! net_send returned %08x\n\n", ret );
        goto exit;
    }

    /*
     * 6. Get the client's public value: Yc = G ^ Xc mod P
     */
    printf( "\n  . Receiving the client's public value" );
    fflush( stdout );

    n = dhm.len;
    if( ( ret = net_recv( client_fd, buf, &n ) ) != 0 )
    {
        printf( " failed\n  ! net_recv returned %08x\n\n", ret );
        goto exit;
    }

    if( ( ret = dhm_read_public( &dhm, buf, n ) ) != 0 )
    {
        printf( " failed\n  ! net_recv returned %08x\n\n", ret );
        goto exit;
    }

    /*
     * 7. Derive the shared secret: K = Ys ^ Xc mod P
     */
    printf( "\n  . Shared secret: " );
    fflush( stdout );

    if( ( ret = dhm_calc_secret( &dhm, buf, &n ) ) != 0 )
    {
        printf( " failed\n  ! dhm_calc_secret returned %08x\n\n", ret );
        goto exit;
    }

    for( n = 0; n < 16; n++ )
        printf( "%02x", buf[n] );

    /*
     * 8. Setup the AES-256 encryption key
     *
     * This is an overly simplified example; best practice is
     * to hash the shared secret with a random value to derive
     * the keying material for the encryption/decryption keys
     * and MACs.
     */
    printf( "...\n  . Encrypting and sending the ciphertext" );
    fflush( stdout );

    aes_set_key( &aes, buf, 256 );
    memcpy( buf, PLAINTEXT, 16 );
    aes_encrypt( &aes, buf, buf );

    n = 16;
    if( ( ret = net_send( client_fd, buf, &n ) ) != 0 )
    {
        printf( " failed\n  ! net_send returned %08x\n\n", ret );
        goto exit;
    }

    printf( "\n\n" );

exit:

    net_close( client_fd );
    rsa_free( &rsa );
    dhm_free( &dhm );

#ifdef WIN32
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
